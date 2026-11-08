"""Pack reviewed v4 PNGs into local runtime atlases. Requires Pillow."""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SEASONS = ("Xuan", "Ha", "Thu", "Dong")
DIRECTIONS = ("e", "se", "s", "sw", "w", "nw", "n", "ne")

def pack(entries, destination, name):
    width = 2048
    padding = 2
    x = y = row_height = 0
    placed = []
    for key, path, pivot, stature in entries:
        with Image.open(path) as original:
            image = original.convert("RGBA")
        if image.width + padding * 2 > width:
            raise ValueError(f"Image too wide: {path}")
        bounds = image.getchannel("A").point(lambda a: 255 if a >= 32 else 0).getbbox()
        if bounds is None:
            raise ValueError(f"Empty sprite: {path}")
        if x + image.width + padding * 2 > width:
            x = 0
            y += row_height
            row_height = 0
        placed.append((key, path, image, bounds, x + padding, y + padding, pivot, stature))
        x += image.width + padding * 2
        row_height = max(row_height, image.height + padding * 2)
    height = 1 << max(0, (y + row_height - 1).bit_length())
    if height > 4096:
        raise ValueError("Atlas exceeds review budget")
    atlas = Image.new("RGBA", (width, height))
    records = []
    for key, path, image, bounds, x, y, pivot, stature in placed:
        atlas.paste(image, (x, y))
        w, h = image.size
        # Extrude edge pixels without changing the PNG source.
        for offset in range(1, padding + 1):
            atlas.paste(image.crop((0, 0, w, 1)), (x, y - offset))
            atlas.paste(image.crop((0, h - 1, w, h)), (x, y + h - 1 + offset))
            atlas.paste(image.crop((0, 0, 1, h)), (x - offset, y))
            atlas.paste(image.crop((w - 1, 0, w, h)), (x + w - 1 + offset, y))
        l, t, r, b = bounds
        px, py = pivot if pivot is not None else ((l + r) / 2, b)
        records.append((key, x, y, w, h, l, t, r-l, b-t, stature or b-t, px, py))
    atlas.save(destination / f"{name}.png")
    with (destination / f"{name}.csv").open("w", encoding="utf-8", newline="") as output:
        writer = csv.writer(output)
        writer.writerow(("id","x","y","w","h","left","top","cw","ch","reference","px","py"))
        writer.writerows(records)
    return len(records)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--configuration", choices=("Debug","Release"), default="Release")
    args = parser.parse_args()
    source = ROOT / "AssetLibrary_Review/DonGian_v4/MyFarmEngine_Assets"
    destination = ROOT / f"out/build/phase-f-{args.configuration.lower()}/Assets/LocalV4"
    destination.mkdir(parents=True, exist_ok=True)
    hashes = {}
    def entry(key, relative, pivot=None, stature=None):
        path = source / relative
        hashes[relative] = hashlib.sha256(path.read_bytes()).hexdigest()
        return key, path, pivot, stature
    masks = json.loads((source / "QUY_TAC_TILE.json").read_text(encoding="utf-8-sig"))["valid_masks"]
    totals = {}
    for number, season in enumerate(SEASONS):
        entries = []
        for mask in masks:
            entries.append(entry(f"land_{mask:03d}", f"Nuoc/{season}/BoAo/boao_co_{mask:03d}.png"))
            entries.append(entry(f"water_{mask:03d}", f"Nuoc/{season}/BoAo/boao_nuoc_{mask:03d}.png"))
        entries.append(entry("grass", f"Co/{season}/co_lap_v3.png"))
        entries.append(entry("path", f"Duong/{season}/duong_lap_01.png"))
        for wet in ("kho","uot"):
            for part in ("dau","giua","cuoi","don"):
                entries.append(entry(f"bed_{wet}_{part}", f"Luong/{season}/luong_{wet}_{part}.png"))
        totals[f"terrain{number}"] = pack(entries,destination,f"terrain{number}")
    entries = []
    stature = 0
    for path in (source / "NhanVat").rglob("*.png"):
        with Image.open(path) as image:
            box = image.getchannel("A").point(lambda a: 255 if a >= 32 else 0).getbbox()
            if box: stature = max(stature,box[3]-box[1])
    for direction in DIRECTIONS:
        entries.append(entry(f"actor_{direction}_idle",f"NhanVat/DungTam/player_{direction}_idle_00.png",(96,180),stature))
        for frame, original in enumerate(("00","02")):
            entries.append(entry(f"actor_{direction}_walk_{frame}",f"NhanVat/DiBo/player_{direction}_walk_{original}.png",(96,180),stature))
    decorations = {
        "house":"Nha/farmhouse_base.png", "shop":"Nha/grocery_store_base.png",
        "fence":"Nha/fence_base.png", "rock":"Da/da_reu_lon.png",
        "bush":"Cay/Bui/bui_qua_do.png", "chest":"DoVat/thung_go.png",
        "sign":"DoVat/bien_chi_duong.png", "tree":"Cay/CayNon/cay_non_04.png", "can":"DoVat/binh_tuoi.png",
        "seeds":"DoVat/bao_hat.png"
    }
    # Named assets must exist, never substitute an unrelated file.
    for key, relative in decorations.items():
        if (source / relative).is_file():
            entries.append(entry(key,relative))
    totals["sprites"] = pack(entries,destination,"sprites")
    (destination / "provenance.json").write_text(json.dumps({"source":"DonGian_v4","sha256":hashes,"counts":totals},indent=2),encoding="utf-8")
    print(destination)
    print(totals)

if __name__ == "__main__":
    main()
