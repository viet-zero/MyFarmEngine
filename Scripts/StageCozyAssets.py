"""Stage generated cozy assets; source images remain unchanged. Requires Pillow."""
import argparse
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
    (destination/'provenance.json').write_text(json.dumps(hashes,indent=2),encoding='utf-8')
    print(destination)

if __name__=='__main__': main()
