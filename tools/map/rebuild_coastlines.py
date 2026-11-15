#!/usr/bin/env python3
"""Rebuild playable provinces. Offline tooling only: pip install shapely.

Coastlines are geographic; province partitions are deliberately approximate game
regions, not a claim to historical administrative boundaries. IDs and gameplay
metadata come from the existing map.json and are preserved.
"""
import json
import math
from pathlib import Path

from shapely.geometry import Polygon, Point, box, shape
from shapely.ops import unary_union

ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path(__file__).with_name("eastern_med_land.geojson")
land = shape(json.loads(SOURCE.read_text())["geometry"])
remaining = land
regions = {}


def take(province_id, mask):
    global remaining
    regions[province_id] = remaining.intersection(mask)
    remaining = remaining.difference(mask)


def island_at(lon, lat):
    return next(part for part in land.geoms if part.covers(Point(lon, lat)))


# Keep true island silhouettes; several Aegean islands share the Chios province.
take(18, island_at(14, 37.5))
take(17, island_at(25, 35.15))
take(13, island_at(33.2, 35))
take(12, unary_union([island_at(26.0, 38.4), island_at(26.25, 39.2),
                     island_at(26.8, 37.75), island_at(26.15, 37.6)]))
take(22, Polygon([(12, 37.8), (18.7, 37.8), (18.7, 40.5),
                  (17.9, 41), (16.7, 41.8), (16, 42.2),
                  (15.2, 43), (14.8, 44), (12, 44)]))

# The European side of the Aegean, Dardanelles, and Bosporus.
europe = Polygon([(12, 35.8), (24.5, 35.8), (26, 39.5),
                  (26.15, 40), (26.7, 40.45), (27.5, 40.65),
                  (28.95, 40.98), (29.04, 41.06), (29.12, 41.23),
                  (28.6, 41.6), (28.1, 42), (28.5, 44), (12, 44)])
europe_remaining = remaining.intersection(europe)
remaining = remaining.difference(europe)
asia_remaining = remaining
remaining = europe_remaining
take(0, Polygon([(27.6, 40), (29.3, 40), (29.3, 41.7),
                 (28.3, 41.55), (28.05, 41.3), (27.8, 41.15)]))
take(16, Polygon([(19, 35.8), (24.5, 35.8), (24.5, 37.5),
                  (23.1, 37.9), (22.95, 38), (22.7, 38.08),
                  (22.2, 38.25), (21.65, 38.32), (21.2, 38.4),
                  (20.8, 38.2), (19, 38.2)]))
take(9, Polygon([(18, 35), (26, 35), (26, 38.85), (24, 38.85),
                 (23.3, 38.7), (22.8, 38.85), (22.2, 39.05),
                 (21.5, 38.95), (20.5, 39.1), (18, 39.1)]))
take(3, Polygon([(12, 35), (20.5, 35), (20.5, 38.8), (21.2, 39.1),
                 (21.45, 39.6), (21.2, 40), (21.4, 40.4),
                 (20.9, 40.8), (12, 41)]))
take(10, Polygon([(20, 38), (26, 38), (26, 39.7), (24, 39.9),
                  (23.1, 40.1), (22.7, 40.05), (22.3, 40.25),
                  (21.8, 40.15), (21.2, 40.3), (20, 40.1)]))
take(4, Polygon([(24.25, 39.5), (30, 39.5), (30, 42.2),
                 (27.8, 42.1), (27, 41.95), (26.4, 41.85),
                 (25.7, 41.7), (25, 41.75), (24.5, 41.5)]))
take(1, Polygon([(12, 39), (26, 39), (26, 41.4), (24.5, 41.5),
                 (23.8, 41.4), (23, 41.55), (22.5, 41.4),
                 (21.7, 41.65), (21, 41.5), (20, 41.8), (12, 41.8)]))
take(2, box(12, 25, 43, 44))
remaining = asia_remaining

# North Africa, Sinai, and the Levant. Shared cuts are applied once, so adjacent
# provinces use exactly the same vertices, including the curved inland borders.
take(21, Polygon([(12, 25), (25, 25), (24.8, 26.2), (25.1, 27.1),
                  (24.9, 28), (25.2, 28.9), (25, 29.7),
                  (25.15, 30.4), (24.9, 31.2), (25, 34), (12, 34)]))
take(20, Polygon([(24, 25), (34.8, 25), (34.8, 29.5),
                  (34.4, 30), (34.3, 30.5), (34.25, 31),
                  (34.2, 31.3), (33.8, 31.6), (32, 33), (24, 34)]))
take(19, Polygon([(34.8, 25), (43, 25), (43, 31.3), (40, 31.4),
                  (38.5, 31.6), (37.4, 31.4), (36.8, 31.65),
                  (36.3, 31.4), (35.8, 30.9), (35.2, 29.5),
                  (34.8, 29.5)]))
take(15, Polygon([(32, 29), (35.8, 29), (36.5, 31), (36.7, 32),
                  (36.5, 32.6), (36.2, 33.2), (35.8, 33.4), (32, 33.4)]))
take(14, Polygon([(32, 31), (36.5, 31), (38.5, 32), (38.9, 33.3),
                  (38.6, 34.2), (38.8, 35), (38.3, 35.7),
                  (37.6, 36.05), (37, 36.3), (36.3, 36.15), (32, 36.3)]))
take(23, Polygon([(36.5, 25), (43, 25), (43, 38.2), (41.5, 38.1),
                  (40.8, 37.8), (40, 37.95), (39.3, 37.6),
                  (38.8, 37.7), (38.2, 37.3), (37.8, 36.9), (37.6, 35)]))
take(11, Polygon([(29, 34), (39, 34), (39, 36.8), (38, 37.2),
                  (37.4, 37.3), (36.8, 37.65), (36, 37.85),
                  (35.3, 37.65), (34.6, 37.35), (33.8, 37.2),
                  (33, 36.8), (32, 36.7), (29, 36.8)]))
take(5, Polygon([(30.5, 44), (43, 44), (43, 40), (41, 40.25),
                 (39.5, 40.1), (38, 40.4), (36.6, 40.1),
                 (35.5, 40.2), (34.3, 40.55), (33, 40.4),
                 (31.8, 40.65), (30.8, 41.1)]))
take(8, Polygon([(34.2, 35), (43, 35), (43, 41), (36.5, 41),
                 (36, 40), (35.7, 39.6), (35.2, 39.3),
                 (35.4, 38.8), (34.9, 38.3), (34.6, 37.8), (34.2, 37.3)]))
take(6, Polygon([(30.8, 37), (36, 37), (37, 44), (30.8, 44),
                 (31.3, 40.7), (31.1, 40.1), (31.5, 39.7),
                 (31.2, 39.2), (31.4, 38.6), (31, 38.1)]))
take(7, box(12, 25, 43, 44))

# Equirectangular projection at 35 degrees N, uniformly fitted to the actual
# 950x740 display. Store in legacy 940x880 map coordinates for input compatibility.
COS_LAT = math.cos(math.radians(35))
SCALE = min(950 / (31 * COS_LAT), 740 / 19)
PAD_X = (950 - 31 * COS_LAT * SCALE) / 2
PAD_Y = (740 - 19 * SCALE) / 2


def project(lon, lat):
    return {"x": round((PAD_X + (lon - 12) * COS_LAT * SCALE) * 940 / 950),
            "y": round((PAD_Y + (44 - lat) * SCALE) * 880 / 740)}


def polygons(geometry):
    if geometry.is_empty:
        return []
    if geometry.geom_type == "Polygon":
        return [geometry]
    if hasattr(geometry, "geoms"):
        return [p for g in geometry.geoms for p in polygons(g)]
    return []


def ring(part):
    result = []
    for lon, lat in list(part.exterior.coords)[:-1]:
        pt = project(lon, lat)
        if not result or pt != result[-1]:
            result.append(pt)
    if len(result) > 1 and result[0] == result[-1]:
        result.pop()
    return result


# A coastline can cross an inland cut several times. Reassign detached mainland
# pieces to the adjacent province's main body instead of treating them as islands.
mainlands = unary_union([p for p in land.geoms if p.area > 3])
for _ in range(24):
    moved = False
    for province_id, region in list(regions.items()):
        pieces = sorted(polygons(region.intersection(mainlands)), key=lambda p: p.area, reverse=True)
        for fragment in pieces[1:]:
            if fragment.area < 1e-10:
                continue
            candidates = {}
            for other_id, other in regions.items():
                if other_id == province_id:
                    continue
                other_parts = polygons(other.intersection(mainlands))
                if not other_parts:
                    continue
                main_body = max(other_parts, key=lambda p: p.area)
                shared = fragment.boundary.intersection(main_body.boundary).length
                if shared > 1e-8:
                    candidates[other_id] = shared
            assert candidates, (province_id, fragment.bounds)
            recipient = max(candidates, key=candidates.get)
            regions[province_id] = regions[province_id].difference(fragment)
            regions[recipient] = regions[recipient].union(fragment)
            moved = True
    if not moved:
        break
else:
    raise AssertionError("Mainland fragment repair did not converge")
for province_id, region in regions.items():
    assert len([p for p in polygons(region.intersection(mainlands)) if p.area > 1e-8]) <= 1, province_id

# Small real islands belong wholly to one province, never sliced by inland cuts.
for island in land.geoms:
    if island.area > 3:
        continue
    owner = max(regions, key=lambda k: regions[k].intersection(island).area)
    for province_id in regions:
        regions[province_id] = regions[province_id].difference(island)
    regions[owner] = regions[owner].union(island)


LABELS = {0: (29.15, 41.8), 1: (21.7, 41), 2: (24.8, 43),
          3: (20, 39.65), 4: (26.25, 41.25), 5: (36.5, 41.5),
          6: (33.25, 39.4), 7: (29.5, 38.7), 8: (38.6, 39),
          9: (22.5, 38.55), 10: (24.9, 39.8), 11: (35.2, 37),
          12: (25.3, 38.35), 13: (33.2, 35), 14: (37.1, 34.7),
          15: (35.2, 32.2), 16: (22.1, 37.2), 17: (25, 34.65),
          18: (14.1, 37.4), 19: (39.3, 28.4), 20: (29.5, 28.1),
          21: (19.8, 28.3), 22: (16, 40), 23: (41.2, 34.9)}

assert remaining.is_empty
assert len(regions) == 24
coverage = unary_union(list(regions.values()))
assert land.symmetric_difference(coverage).area < 1e-8
assert abs(sum(g.area for g in regions.values()) - coverage.area) < 1e-8

path = ROOT / "data/map.json"
data = json.loads(path.read_text())
for province in data["provinces"]:
    parts = sorted(polygons(regions[province["id"]]), key=lambda g: g.area, reverse=True)
    rings = []
    for part in parts:
        points = ring(part)
        if len(points) < 3:
            continue
        # Integer map coordinates can collapse narrow inlets to a touching edge.
        # Split those pinches into valid pieces rather than self-intersections.
        snapped = Polygon([(p["x"], p["y"]) for p in points]).buffer(0)
        for piece in polygons(snapped):
            if piece.area >= 1:
                rings.append([{"x": round(x), "y": round(y)}
                              for x, y in list(piece.exterior.coords)[:-1]])
    assert rings, province["name"]
    for r in rings:
        assert Polygon([(p["x"], p["y"]) for p in r]).is_valid, province["name"]
    province["polygon"] = rings[0]
    province["islands"] = rings[1:]
    province["label"] = project(*LABELS[province["id"]])
data["geography"] = {"source": "Natural Earth 1:50m land (public domain)",
                     "bounds": [12, 25, 43, 44],
                     "projection": "Equirectangular, standard parallel 35 N",
                     "note": "Approximate gameplay regions; existing IDs and connections preserved."}
path.write_text(json.dumps(data, indent=2) + "\n")
print(f"Wrote {len(regions)} provinces from real coastlines to {path}")
