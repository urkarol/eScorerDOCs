"""Rebuild embedded template data. Requires PyMuPDF and fontTools (build only)."""
import fitz,re,json,sys
from pathlib import Path
from fontTools.ttLib import TTFont
root=Path(__file__).resolve().parent
source=Path(sys.argv[1]) if len(sys.argv)>1 else root/'lineup_source.pdf'
p=fitz.open(source);page=p[0]
rects=[d['rect'] for d in page.get_drawings() if len(d['items'])==1 and d['items'][0][0]=='re' and abs(d['rect'].width-19.802)<.01 and abs(d['rect'].height-50.918)<.01]
assert len(rects)==10
pattern=rb'q\s+1 0 0 1 [-\d.]+ [-\d.]+ cm\s+/CS0 CS\s+0\.137 0\.122 0\.125 SCN\s+0\.249 w\s+3\.864 M\s+/GS1 gs\s+0 0 m\s+4\.659 0 l\s+S\s+Q'
total=0
for x in page.get_contents():
 data,n=re.subn(pattern,b'',p.xref_stream(x));total+=n
 if n:p.update_stream(x,data)
assert total==20
spans=[s for b in page.get_text('dict')['blocks'] for l in b.get('lines',[]) for s in l['spans'] if 'I M E' in s['text']]
assert len(spans)==10
for s in spans:page.add_redact_annot(fitz.Rect(s['bbox']),fill=False,cross_out=False)
page.apply_redactions(images=0,graphics=0,text=0)
fontpath='/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
page.insert_font(fontname='TeamCode',fontfile=fontpath)
p.save(root/'lineup_template.pdf',garbage=4,deflate=True)
p=fitz.open(root/'lineup_template.pdf');page=p[0];data=(root/'lineup_template.pdf').read_bytes()
new=p.xref_length();pageobj=p.xref_object(page.xref,compressed=True)
pageobj=re.sub(r'/Contents\s*(\d+\s+0\s+R)',lambda m:'/Contents['+m[1]+' '+str(new)+' 0 R]',pageobj)
assert str(new)+' 0 R' in pageobj
prev=int(re.findall(rb'startxref\s+(\d+)',data)[-1]);rootref=p.xref_get_key(-1,'Root')[1]
font=TTFont(fontpath);cmap=font.getBestCmap();chars=' ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';glyphs=[]
for ch in chars:
 name=cmap[ord(ch)];g=font['glyf'][name];glyphs.append([font.getGlyphID(name),font['hmtx'].metrics[name][0],getattr(g,'xMin',0),getattr(g,'yMin',0),getattr(g,'xMax',0),getattr(g,'yMax',0)])
with (root/'lineup_data.hpp').open('w') as o:
 o.write('// Generated from lineup_template.pdf. Do not edit.\n#pragma once\nnamespace lineupData {\n')
 o.write('inline const unsigned char pdf[]={\n')
 for i in range(0,len(data),24):o.write(','.join(str(v) for v in data[i:i+24])+',\n')
 o.write('};\n')
 for key,value in [('pageObject',page.xref),('streamObject',new),('previousXref',prev),('unitsPerEm',font['head'].unitsPerEm)]:o.write(f'inline constexpr int {key}={value};\n')
 o.write('inline const char* pageDictionary='+json.dumps(pageobj)+';\n')
 o.write('inline const char* rootReference='+json.dumps(rootref)+';\n')
 o.write('inline const char* chars='+json.dumps(chars)+';\n')
 o.write('inline constexpr int glyphs[][6]={'+','.join('{'+','.join(map(str,g))+'}' for g in glyphs)+'};\n')
 o.write('inline constexpr double centers[][2]={'+','.join('{%.8f,%.8f}'%((r.x0+r.x1)/2,(r.y0+r.y1)/2) for r in rects)+'};\n}\n')
print('Embedded template:',len(data),'bytes; page',page.xref,'new stream',new)
