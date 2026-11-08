"""Inventory tracked project files plus new source files, excluding local review archives."""
from pathlib import Path
import csv
import subprocess

ROOT=Path(__file__).resolve().parents[1]

def main():
    tracked=subprocess.check_output(['git','ls-files','-z'],cwd=ROOT).decode('utf-8').split(chr(0))
    extra=subprocess.check_output(['git','ls-files','--others','--exclude-standard','-z'],cwd=ROOT).decode('utf-8').split(chr(0))
    paths=sorted({p for p in tracked+extra if p and not p.startswith('AssetLibrary_Review/')})
    with (ROOT/'Docs/FILE_INDEX.csv').open('w',encoding='utf-8-sig',newline='') as out:
        writer=csv.writer(out)
        writer.writerow(('path','group','bytes','lines_if_text','empty','note'))
        for relative in paths:
            path=ROOT/relative
            if not path.is_file(): continue
            data=path.read_bytes()
            text_file=path.suffix.lower() in ('.h','.hpp','.cpp','.c','.md','.txt','.csv','.json','.ps1','.py','.glsl','.vert','.frag','.cmake') or path.name in ('.gitignore','CMakeLists.txt')
            count=len(data.splitlines()) if text_file else ''
            note='EMPTY: not evidence of an implemented feature' if not data else 'See PROJECT_HANDOVER.md; presence does not imply runtime integration'
            writer.writerow((relative,relative.split('/')[0],len(data),count,not bool(data),note))
    print('Wrote Docs/FILE_INDEX.csv')

if __name__=='__main__': main()
