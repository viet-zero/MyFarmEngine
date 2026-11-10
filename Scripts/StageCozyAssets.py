"""Stage generated cozy assets; source images remain unchanged. Requires Pillow."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
from StageV4Assets import pack

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--configuration',choices=('Debug','Release'),default='Release')
    args=parser.parse_args()
    source=ROOT/'Assets/Workspace/CozyV1'
    destination=ROOT/f'out/build/phase-f-{args.configuration.lower()}/Assets/Cozy'
    destination.mkdir(parents=True,exist_ok=True)
    hashes={}
    # Split atlases to keep each texture within 2048x4096, without resizing sources.
    for group,names in {'ground':('grass','water','meadow'),'props':('tree','house')}.items():
        entries=[]
        for name in names:
            path=source/(name+'-v1.png')
            hashes[path.name]=hashlib.sha256(path.read_bytes()).hexdigest()
            entries.append((name,path,None,None))
        pack(entries,destination,group,width=4096 if group=="ground" else 2048,padding=16)
    detail_source=ROOT/'Assets/Workspace/CozyV2'
    if detail_source.is_dir():
        entries=[]
        roles=json.loads((detail_source/'asset_roles.json').read_text(encoding='utf-8-sig'))
        used=set()
        for record in roles['assets']:
            key,name=record['id'],record['file']
            path=(detail_source/name).resolve()
            if path.parent!=detail_source.resolve() or key in used:
                raise ValueError('Invalid/duplicate asset role')
            used.add(key)
            hashes[name]=hashlib.sha256(path.read_bytes()).hexdigest()
            entries.append((key,path,None,None))
        if used!={'road','fence','house_open'}: raise ValueError('Required logical roles changed')
        from PIL import Image
        with Image.open(source/'house-v1.png') as closed, Image.open(detail_source/'building_farmhouse_door_open_v02.png') as opened:
            if closed.size!=opened.size: raise ValueError('House state canvases must match')
        pack(entries,destination,'details',padding=16)
        csv_path=destination/'details.csv'
        with csv_path.open(encoding='utf-8',newline='') as file:
            reader=csv.DictReader(file);fields=reader.fieldnames;rows=list(reader)
        frames={row['id']:row for row in rows}
        for region in roles.get('regions',[]):
            if region['id'] in frames: raise ValueError('Duplicate region')
            parent=frames[region['parent']];x,y,w,h=region['rect']
            if min(x,y)<0 or min(w,h)<=0 or x+w>int(parent['w']) or y+h>int(parent['h']):
                raise ValueError('Region outside source frame')
            row=dict(zip(fields,[region['id'],int(parent['x'])+x,int(parent['y'])+y,w,h,0,0,w,h,h,w/2,h]))
            rows.append(row);frames[region['id']]=row
        with csv_path.open('w',encoding='utf-8',newline='') as file:
            writer=csv.DictWriter(file,fieldnames=fields);writer.writeheader();writer.writerows(rows)
    (destination/'provenance.json').write_text(json.dumps(hashes,indent=2),encoding='utf-8')
    print(destination)

if __name__=='__main__': main()
