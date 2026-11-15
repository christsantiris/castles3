# Coastline map

The playable map uses Natural Earth **1:50m land v4.1.0**, in the public domain:
https://www.naturalearthdata.com/downloads/50m-physical-vectors/50m-land/

`eastern_med_land.geojson` is an offline crop of that dataset to 12–43° E,
25–44° N, simplified with a 0.012° tolerance while preserving topology.
It was extracted from https://naturalearth.s3.amazonaws.com/50m_physical/ne_50m_land.zip.

`rebuild_coastlines.py` clips this land into the existing 24 gameplay regions.
These inland boundaries are approximate game regions, not historical borders.
IDs, names, owners, resources, and neighbor lists are retained from `data/map.json`.
Sea connections still follow those original neighbor lists.

Detached mainland pieces created by coastal/inland intersections are reassigned
to the adjoining province with the longest shared border to its main body.
Each mainland province must then have one continuous body. Real islands are
assigned whole to one province; inland cuts cannot divide an island.

The projection is equirectangular at 35° N and fitted uniformly to the 950×740
map viewport before conversion to legacy 940×880 map coordinates. This accounts
for the renderer's different X/Y scales and preserves geographic proportions.
Islands use separate polygons; inland lake holes are omitted at this display scale.
Label anchors are explicit preferences. At runtime, label layout uses measured
font dimensions and a raster area check to fit the entire padded text rectangle
inside its province. Names that cannot fit use nonoverlapping sea callouts with
leader lines and endpoint markers. Text has a dark halo for contrast, including
on selected provinces. Layout is cached until the geometry, names, or font change.
Static geometry and labels come from map.json when loading a game; save files
continue to store mutable province ownership and visibility by ID.

To adjust boundaries or label positions and rebuild (no network needed):

```sh
python3 -m venv /tmp/castles3-map-tools
/tmp/castles3-map-tools/bin/pip install shapely
/tmp/castles3-map-tools/bin/python tools/map/rebuild_coastlines.py
make build
```

Shapely is needed only to regenerate the checked-in JSON. The game has no new
runtime dependencies. Very small islands below one map-coordinate square are
omitted. The generator checks that regions cover the source land without overlap
before integer rounding and that exported polygon rings are valid.
