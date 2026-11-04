"""Read-only character artwork audit. Requires Python + Pillow; writes derived review evidence only."""
import argparse
import csv
import hashlib
import math
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/reviews/p0-001')
    args = parser.parse_args()
    output = args.output.resolve()
    if output == ROOT or any(output == ROOT / part or (ROOT / part) in output.parents
                             for part in ('Assets', 'Gameplay', 'Engine', 'Game', 'ThirdParty')):
        parser.error('Output must be a separate review directory, outside source/assets.')
    output.mkdir(parents=True, exist_ok=True)
    with (ROOT / 'Gameplay/Data/Assets.csv').open(encoding='utf-8-sig', newline='') as f:
        records = list(csv.DictReader(f))
    frames, metrics, hashes = [], [], []
    for record in records:
        if record['kind'] != 'character' and not (record['kind'] == 'animal' and record['status'] == 'ready'):
            continue
        relative = record['export_path'] if record['status'] == 'ready' else record['source']
        path = (ROOT / 'Assets' / relative).resolve()
        if (ROOT / 'Assets') not in path.parents:
            raise ValueError('Asset path escapes root')
        before = hashlib.sha256(path.read_bytes()).hexdigest()
        with Image.open(path) as image:
            image.load()
            rgba = image.convert('RGBA')
        w, h, cols = (int(record[k]) for k in ('frame_width', 'frame_height', 'columns'))
        local = []
        for n in range(int(record['frame_count'])):
            index = int(record['first_frame']) + n
            x = int(record['margin_x']) + index % cols * (w + int(record['spacing_x']))
            y = int(record['margin_y']) + index // cols * (h + int(record['spacing_y']))
            if x < 0 or y < 0 or x+w > rgba.width or y+h > rgba.height:
                raise ValueError('Frame outside image: ' + record['asset_id'])
            frame = rgba.crop((x, y, x+w, y+h))
            alpha = frame.getchannel('A')
            bounds = alpha.point(lambda a: 255 if a >= 32 else 0).getbbox()
            if bounds is None:
                raise ValueError('Empty frame: ' + record['asset_id'])
            l,t,r,b = bounds
            histogram = alpha.histogram()
            metric = dict(asset_id=record['asset_id'],status=record['status'],frame=n,
                          frame_width=w,frame_height=h,left=l,top=t,right_exclusive=r,bottom_exclusive=b,
                          content_height=b-t,auto_pivot_x=(l+r)/2,auto_pivot_y=b,
                          alpha_min=alpha.getextrema()[0],alpha_max=alpha.getextrema()[1],
                          soft_alpha_pixels=sum(histogram[1:255]),
                          touches_cell_edge=l==0 or t==0 or r==w or b==h,
                          metadata_anchor_x=record['anchor_x'],metadata_anchor_y=record['anchor_y'])
            local.append((frame, bounds, metric))
        reference = max(m['content_height'] for _,_,m in local)
        for frame,bounds,metric in local:
            metric['reference_height'] = reference
            metric['world_scale'] = (72 if record['kind']=='animal' else 132)/reference
            metrics.append(metric)
            frames.append((frame,bounds,metric,record['kind']))
        if hashlib.sha256(path.read_bytes()).hexdigest() != before:
            raise RuntimeError('Input changed during audit: ' + str(path))
        hashes.append(dict(asset_id=record['asset_id'],path=relative,sha256=before))
    for name, rows in [('frames.csv',metrics),('inputs.csv',hashes)]:
        with (output/name).open('w',encoding='utf-8',newline='') as f:
            writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
    for group, selected in [('runtime',[v for v in frames if v[2]['status']=='ready']),
                            ('library',[v for v in frames if v[2]['status']!='ready'])]:
        for theme,bg,ink in [('light',(236,234,223),(20,20,20)),('dark',(30,38,46),(245,245,245))]:
            width,height,cols=220,205,7
            board=Image.new('RGB',(cols*width,40+math.ceil(len(selected)/cols)*height),bg)
            draw=ImageDraw.Draw(board)
            draw.text((8,8),group+' / '+theme+' | 132 world-pixel human, 72 pet | cyan = AUTO bounds pivot, NOT artist-approved',fill=ink)
            for i,(frame,bounds,m,kind) in enumerate(selected):
                x=i%cols*width;y=40+i//cols*height;ground=y+151
                l,t,r,b=bounds;s=m['world_scale']
                # Preview is derived only: same sheet reference height and uniform scale as runtime.
                crop=frame.crop(bounds)
                resample=Image.Resampling.NEAREST if group=='library' else Image.Resampling.LANCZOS
                crop=crop.resize((max(1,round((r-l)*s)),max(1,round((b-t)*s))),resample)
                left=round(x+width/2-(r-l)*s/2)
                board.paste(crop,(left,ground-crop.height),crop)
                draw.line((x+8,ground,x+width-8,ground),fill=(110,120,125))
                draw.line((x+width//2-5,ground,x+width//2+5,ground),fill=(0,185,220),width=2)
                draw.line((x+width//2,ground-5,x+width//2,ground+5),fill=(0,185,220),width=2)
                label=m['asset_id'].replace('library_snhr_v11_characters_character_','lib_').replace('_sprite','')
                draw.text((x+5,y+159),label,fill=ink)
                draw.text((x+5,y+174),'frame '+str(m['frame'])+' | h='+str(m['content_height'])+(' EDGE' if m['touches_cell_edge'] else ''),fill=ink)
            board.save(output/(group+'-'+theme+'.png'))
    # Show actual clip orientation, including CSV flip_x, separately from raw cells.
    by_id = {r['asset_id']:r for r in records}
    with (ROOT/'Gameplay/Data/VisualClips.csv').open(encoding='utf-8-sig',newline='') as f:
        clips=[c for c in csv.DictReader(f) if c['profile'] in ('player_linh','player_an') and c['state'] in ('idle','walk','action')]
    for theme,bg,ink in [('light',(236,234,223),(20,20,20)),('dark',(30,38,46),(245,245,245))]:
        board=Image.new('RGB',(4*260,45+6*205),bg);draw=ImageDraw.Draw(board)
        draw.text((8,8),'ACTUAL CSV clip first frame + flip_x | cyan = auto pivot | labels = declared directions',fill=ink)
        for i,c in enumerate(clips):
            r=by_id[c['asset_id']];n=int(c['first_frame'])-int(r['first_frame'])
            frame,bounds,m,_=next(v for v in frames if v[2]['asset_id']==c['asset_id'] and v[2]['frame']==n)
            l,t,right,b=bounds;crop=frame.crop(bounds);scale=m['world_scale']
            crop=crop.resize((max(1,round((right-l)*scale)),max(1,round((b-t)*scale))),Image.Resampling.LANCZOS)
            if c['flip_x']=='1': crop=crop.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
            x=i%4*260;y=45+i//4*205;ground=y+150
            board.paste(crop,(x+130-crop.width//2,ground-crop.height),crop)
            draw.line((x+8,ground,x+252,ground),fill=(110,120,125))
            draw.line((x+125,ground,x+135,ground),fill=(0,185,220),width=2)
            draw.text((x+5,y+158),c['profile']+' '+c['state']+' '+c['direction'],fill=ink)
            draw.text((x+5,y+174),'source frame '+c['first_frame']+' flip_x='+c['flip_x'],fill=ink)
        board.save(output/('clips-'+theme+'.png'))
    # Native-resolution edge samples on real backgrounds; no chroma key or cleanup.
    samples=[('player_linh_idle_dirs',(170,0,430,180)),('player_an_idle_dirs',(190,0,440,180))]
    edge=Image.new('RGB',(1040,810),(80,80,80));draw=ImageDraw.Draw(edge)
    for row,(asset,box) in enumerate(samples):
        record=by_id[asset]
        with Image.open(ROOT/'Assets'/record['export_path']) as image:
            crop=image.convert('RGBA').crop(box)
        for col,color in enumerate([(236,234,223),(30,38,46)]):
            tile=Image.new('RGBA',crop.size,color+(255,));tile.alpha_composite(crop)
            tile=tile.convert('RGB').resize((crop.width*2,crop.height*2),Image.Resampling.NEAREST)
            edge.paste(tile,(col*520,row*405+30))
        draw.text((8,row*405+8),asset+' source pixels, nearest diagnostic enlargement',fill='white')
    edge.save(output/'alpha-edges.png')
    with Image.open(ROOT/'Assets/Export/Art/World/world_phase_e.png') as image:
        canopy=image.convert('RGB')
    draw=ImageDraw.Draw(canopy)
    with (ROOT/'Gameplay/Data/ForegroundTrees.csv').open(encoding='utf-8-sig',newline='') as f:
        for tree in csv.DictReader(f):
            x,y,w,h=[float(tree[k])/4 for k in ('canopy_x','canopy_y','width','height')]
            draw.rectangle((x,y,x+w,y+h),outline=(255,0,255),width=2)
            draw.ellipse((x,y,x+w,y+h),outline=(0,255,255),width=3)
            draw.text((x,y-14),tree['gameplay_id']+' ellipse proxy',fill=(255,255,255),stroke_width=1,stroke_fill=(0,0,0))
    canopy.save(output/'canopy-proxies.png')
    print(str(output))
    print(f'{len(hashes)} inputs, {len(metrics)} frames; edge flags: '+str(sum(m['touches_cell_edge'] for m in metrics)))

if __name__ == '__main__':
    main()
