#include "geojson_pathfinder/geojson_parser.hpp"

#include "geojson_pathfinder/graph.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace gjp {

namespace {

// Minimal hand-rolled JSON tokenizer + recursive parser sufficient for
// GeoJSON FeatureCollections. We don't materialize a full DOM; instead we
// stream through and extract only what we need (coordinates + geometry type).
class Cursor {
public:
    explicit Cursor(const std::string& src) : src_(src) {}

    [[nodiscard]] bool eof() const { return pos_ >= src_.size(); }
    [[nodiscard]] char peek() const { return src_[pos_]; }
    char get() { return src_[pos_++]; }

    void skip_ws() {
        while (pos_ < src_.size() && std::isspace(static_cast<unsigned char>(src_[pos_]))) {
            ++pos_;
        }
    }

    void expect(char c) {
        skip_ws();
        if (eof() || src_[pos_] != c) {
            std::ostringstream msg;
            msg << "GeoJSON parse error: expected '" << c << "' at position " << pos_;
            throw std::runtime_error(msg.str());
        }
        ++pos_;
    }

    [[nodiscard]] std::string parse_string() {
        skip_ws();
        expect_char('"');
        std::string out;
        while (!eof()) {
            char c = get();
            if (c == '"') return out;
            if (c == '\\' && !eof()) {
                char esc = get();
                switch (esc) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'r': out += '\r'; break;
                    default: out += esc; break;
                }
            } else {
                out += c;
            }
        }
        throw std::runtime_error("GeoJSON parse error: unterminated string");
    }

    [[nodiscard]] double parse_number() {
        skip_ws();
        const std::size_t start = pos_;
        if (!eof() && (peek() == '-' || peek() == '+')) ++pos_;
        while (!eof() && (std::isdigit(static_cast<unsigned char>(peek())) ||
                          peek() == '.' || peek() == 'e' || peek() == 'E' ||
                          peek() == '-' || peek() == '+')) {
            ++pos_;
        }
        return std::stod(src_.substr(start, pos_ - start));
    }

    void skip_value() {
        skip_ws();
        if (eof()) return;
        char c = peek();
        if (c == '{') { skip_object(); return; }
        if (c == '[') { skip_array(); return; }
        if (c == '"') { (void)parse_string(); return; }
        if (c == 't' || c == 'f' || c == 'n') {
            while (!eof() && std::isalpha(static_cast<unsigned char>(peek()))) ++pos_;
            return;
        }
        (void)parse_number();
    }

    void skip_object() {
        expect('{');
        skip_ws();
        if (!eof() && peek() == '}') { ++pos_; return; }
        while (true) {
            (void)parse_string();
            expect(':');
            skip_value();
            skip_ws();
            if (eof()) throw std::runtime_error("GeoJSON parse error: unterminated object");
            if (peek() == ',') { ++pos_; continue; }
            if (peek() == '}') { ++pos_; return; }
            throw std::runtime_error("GeoJSON parse error: expected ',' or '}'");
        }
    }

    void skip_array() {
        expect('[');
        skip_ws();
        if (!eof() && peek() == ']') { ++pos_; return; }
        while (true) {
            skip_value();
            skip_ws();
            if (eof()) throw std::runtime_error("GeoJSON parse error: unterminated array");
            if (peek() == ',') { ++pos_; continue; }
            if (peek() == ']') { ++pos_; return; }
            throw std::runtime_error("GeoJSON parse error: expected ',' or ']'");
        }
    }

private:
    void expect_char(char c) {
        if (eof() || src_[pos_] != c) {
            throw std::runtime_error("GeoJSON parse error: expected character");
        }
        ++pos_;
    }

    const std::string& src_;
    std::size_t pos_{0};
};

// Coordinate key for snapping nearly-identical points. We quantize to ~1e-6
// degrees (~11cm at the equator) to handle floating-point noise in input data.
struct CoordKey {
    std::int64_t lon_q{0};
    std::int64_t lat_q{0};
    bool operator==(const CoordKey& o) const noexcept {
        return lon_q == o.lon_q && lat_q == o.lat_q;
    }
};

struct CoordKeyHash {
    std::size_t operator()(const CoordKey& k) const noexcept {
        return std::hash<std::int64_t>{}(k.lon_q) ^
               (std::hash<std::int64_t>{}(k.lat_q) << 1);
    }
};

CoordKey make_key(const Coordinate& c) {
    constexpr double kScale = 1e6;
    return CoordKey{static_cast<std::int64_t>(c.lon * kScale),
                    static_cast<std::int64_t>(c.lat * kScale)};
}

// Parses an array of two numbers: [lon, lat]. Ignores trailing elements (altitude).
Coordinate parse_coord_pair(Cursor& cur) {
    cur.expect('[');
    Coordinate c;
    c.lon = cur.parse_number();
    cur.skip_ws();
    cur.expect(',');
    c.lat = cur.parse_number();
    cur.skip_ws();
    while (!cur.eof() && cur.peek() == ',') {
        cur.get();
        (void)cur.parse_number();  // altitude or other
        cur.skip_ws();
    }
    cur.expect(']');
    return c;
}

// Parses [[lon,lat], [lon,lat], ...] — currently unused; kept for clarity / future use.
[[maybe_unused]] std::vector<Coordinate> parse_coord_array(Cursor& cur) {
    cur.expect('[');
    std::vector<Coordinate> out;
    cur.skip_ws();
    if (!cur.eof() && cur.peek() == ']') { cur.get(); return out; }
    while (true) {
        out.push_back(parse_coord_pair(cur));
        cur.skip_ws();
        if (cur.peek() == ',') { cur.get(); continue; }
        cur.expect(']');
        return out;
    }
}

}  // namespace

Graph GeoJsonParser::parse(const std::string& geojson_text) {
    Graph g;
    std::unordered_map<CoordKey, NodeId, CoordKeyHash> coord_to_node;

    auto get_or_create_node = [&](const Coordinate& c) -> NodeId {
        const auto key = make_key(c);
        if (auto it = coord_to_node.find(key); it != coord_to_node.end()) {
            return it->second;
        }
        const NodeId id = g.add_node(c);
        coord_to_node.emplace(key, id);
        return id;
    };

    // Walk the JSON looking for "features" array, then for each feature
    // pull out geometry.type and geometry.coordinates.
    Cursor cur(geojson_text);
    cur.expect('{');
    cur.skip_ws();
    if (!cur.eof() && cur.peek() == '}') { return g; }

    while (true) {
        const std::string key = cur.parse_string();
        cur.expect(':');
        if (key != "features") {
            cur.skip_value();
        } else {
            cur.expect('[');
            cur.skip_ws();
            if (!cur.eof() && cur.peek() == ']') { cur.get(); }
            else {
                while (true) {
                    // Parse one feature object: extract geometry.
                    cur.expect('{');
                    std::string geom_type;
                    std::vector<Coordinate> line_coords;
                    Coordinate point_coord{};
                    bool got_point = false;

                    cur.skip_ws();
                    if (!cur.eof() && cur.peek() == '}') { cur.get(); }
                    else {
                        while (true) {
                            const std::string fkey = cur.parse_string();
                            cur.expect(':');
                            if (fkey != "geometry") {
                                cur.skip_value();
                            } else {
                                cur.expect('{');
                                cur.skip_ws();
                                if (!cur.eof() && cur.peek() == '}') { cur.get(); }
                                else {
                                    while (true) {
                                        const std::string gkey = cur.parse_string();
                                        cur.expect(':');
                                        if (gkey == "type") {
                                            geom_type = cur.parse_string();
                                        } else if (gkey == "coordinates") {
                                            cur.skip_ws();
                                            // Peek to decide: Point = [num, num], LineString = [[..], [..]]
                                            // Save position by reading; we'll branch on geom_type known later, so accept both:
                                            // Easiest path — read as array of arrays; if first child isn't an array, treat as point.
                                            // Implementation: check the character after '['
                                            // After skipping ws, look at first non-ws after '['
                                            std::size_t save_pos = 0;
                                            (void)save_pos;
                                            // Inline detection:
                                            cur.skip_ws();
                                            // We need to peek 2 chars ahead. Look for "[[".
                                            // Re-implement with a small lookahead:
                                            // Read first '['
                                            cur.expect('[');
                                            cur.skip_ws();
                                            if (!cur.eof() && cur.peek() == '[') {
                                                // LineString-style: array of coord pairs
                                                line_coords.clear();
                                                if (cur.peek() == ']') { cur.get(); }
                                                else {
                                                    while (true) {
                                                        line_coords.push_back(parse_coord_pair(cur));
                                                        cur.skip_ws();
                                                        if (cur.peek() == ',') { cur.get(); continue; }
                                                        cur.expect(']');
                                                        break;
                                                    }
                                                }
                                            } else {
                                                // Point-style: [lon, lat]
                                                point_coord.lon = cur.parse_number();
                                                cur.skip_ws();
                                                cur.expect(',');
                                                point_coord.lat = cur.parse_number();
                                                cur.skip_ws();
                                                while (!cur.eof() && cur.peek() == ',') {
                                                    cur.get();
                                                    (void)cur.parse_number();
                                                    cur.skip_ws();
                                                }
                                                cur.expect(']');
                                                got_point = true;
                                            }
                                        } else {
                                            cur.skip_value();
                                        }
                                        cur.skip_ws();
                                        if (cur.peek() == ',') { cur.get(); continue; }
                                        cur.expect('}');
                                        break;
                                    }
                                }
                            }
                            cur.skip_ws();
                            if (cur.peek() == ',') { cur.get(); continue; }
                            cur.expect('}');
                            break;
                        }
                    }

                    if (geom_type == "Point" && got_point) {
                        (void)get_or_create_node(point_coord);
                    } else if (geom_type == "LineString" && line_coords.size() >= 2) {
                        for (std::size_t i = 1; i < line_coords.size(); ++i) {
                            NodeId a = get_or_create_node(line_coords[i - 1]);
                            NodeId b = get_or_create_node(line_coords[i]);
                            if (a == b) continue;
                            const double w = Graph::haversine_meters(line_coords[i - 1],
                                                                     line_coords[i]);
                            g.add_edge(a, b, w);
                        }
                    }

                    cur.skip_ws();
                    if (cur.peek() == ',') { cur.get(); continue; }
                    cur.expect(']');
                    break;
                }
            }
        }
        cur.skip_ws();
        if (cur.peek() == ',') { cur.get(); continue; }
        cur.expect('}');
        break;
    }
    return g;
}

Graph GeoJsonParser::parse_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open GeoJSON file: " + path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return parse(ss.str());
}

}  // namespace gjp
