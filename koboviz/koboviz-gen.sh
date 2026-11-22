#!/bin/sh
# ─────────────────────────────────────────────────────────────────────────
# koboviz-gen.sh — ЖИВОЙ генератор статистики и выделений прямо на Kobo.
# Читает свежую KoboReader.sqlite через sqlite3 (собранный под ARM) и заново
# рисует stats.html + highlights.html в .kobo/markups/koboviz/ (эту папку
# nickel не индексирует → не превращается в книги-мусор).
# Вызывается демоном при входе в библиотеку/дом → цифры всегда «в моменте».
#
# Переменные для теста на Маке: KOBOVIZ_SQL, KOBOVIZ_DB, KOBOVIZ_BASE.
# ─────────────────────────────────────────────────────────────────────────
BASE="${KOBOVIZ_BASE:-/mnt/onboard/.kobo/markups/koboviz}"
DB="${KOBOVIZ_DB:-/mnt/onboard/.kobo/KoboReader.sqlite}"
SQL="$KOBOVIZ_SQL"
if [ -z "$SQL" ]; then
  if   [ -x /usr/local/koboviz/sqlite3 ]; then SQL=/usr/local/koboviz/sqlite3
  elif [ -f "$BASE/sqlite3-arm" ]; then cp -f "$BASE/sqlite3-arm" /tmp/koboviz-sqlite3 2>/dev/null; chmod +x /tmp/koboviz-sqlite3 2>/dev/null; SQL=/tmp/koboviz-sqlite3
  else SQL=sqlite3; fi
fi
mkdir -p "$BASE"
q(){ "$SQL" "$DB" "$1" 2>/dev/null; }
B="ContentType=6 AND BookID IS NULL"
HLW="Text IS NOT NULL AND TRIM(Text)<>'' AND LOWER(CAST(Hidden AS TEXT)) NOT IN ('true','1')"
esc(){ printf '%s' "$1" | sed 's/&/\&amp;/g;s/</\&lt;/g;s/>/\&gt;/g'; }  # HTML-escape в shell (для заголовков)

# ── метрики ──
HOURS=$(q "SELECT COALESCE(ROUND(SUM(TimeSpentReading)/3600.0,1),0) FROM content WHERE $B")
DAYS=$(q "SELECT COALESCE(ROUND(SUM(TimeSpentReading)/86400.0,1),0) FROM content WHERE $B")
BOOKS=$(q "SELECT COUNT(*) FROM content WHERE $B")
FIN=$(q "SELECT COUNT(*) FROM content WHERE $B AND ReadStatus=2")
READING=$(q "SELECT COUNT(*) FROM content WHERE $B AND ReadStatus=1")
UNRD=$(q "SELECT COUNT(*) FROM content WHERE $B AND ReadStatus=0")
HL=$(q "SELECT COUNT(*) FROM Bookmark WHERE $HLW")
AUTHORS=$(q "SELECT COUNT(DISTINCT Attribution) FROM content WHERE $B AND Attribution<>''")
LANGS=$(q "SELECT COUNT(DISTINCT substr(Language,1,2)) FROM content WHERE $B AND Language<>''")
COLORS=$(q "SELECT COUNT(DISTINCT Color) FROM Bookmark WHERE $HLW")
WAR=$(q "SELECT COUNT(*) FROM content WHERE $B AND (Title LIKE '%война%' OR Title LIKE '%войн%' OR Title LIKE '%Гитлер%' OR Title LIKE '%арми%' OR Title LIKE '%битв%' OR Title LIKE '%истори%' OR Title LIKE '%атом%' OR Title LIKE '%апокалипс%')")
NOW=$(q "SELECT strftime('%d.%m.%Y %H:%M', 'now','localtime')")

# ── key=value sidecar for the native-view stats-block injector (safe-v3 plugin reads this) ──
STATS_TXT="${STATS_TXT:-/usr/local/koboviz/stats.txt}"
{
  echo "hours=$HOURS"
  echo "days=$DAYS"
  echo "books=$BOOKS"
  echo "finished=$FIN"
  echo "reading=$READING"
  echo "highlights=$HL"
  echo "authors=$AUTHORS"
  echo "langs=$LANGS"
  echo "colors=$COLORS"
  echo "updated=$NOW"
} > "$STATS_TXT.tmp" 2>/dev/null && mv -f "$STATS_TXT.tmp" "$STATS_TXT" 2>/dev/null

# ── highlights sidecar for the native "My Words" injector: one per line "COLOR<TAB>TEXT" (newest first,
#    newlines flattened, capped) ──
HL_TXT="${HL_TXT:-/usr/local/koboviz/highlights.txt}"
q "SELECT COALESCE(Color,0)||char(9)||REPLACE(REPLACE(REPLACE(TRIM(Text),char(10),' '),char(13),' '),char(9),' ')
   FROM Bookmark WHERE $HLW ORDER BY DateCreated DESC LIMIT 300" > "$HL_TXT.tmp" 2>/dev/null \
   && mv -f "$HL_TXT.tmp" "$HL_TXT" 2>/dev/null

# общий CSS (без эмодзи в табах — старый WebKit их не рисует; резиновая ширина под ридер)
CSS='<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#f4efe6;color:#2b2620;font-family:Georgia,"Times New Roman",serif;font-size:24px;line-height:1.5;-webkit-text-size-adjust:100%;word-wrap:break-word}
.wrap{max-width:100%;padding:20px 16px 70px}
.nav{text-align:center;margin-bottom:22px}
.nav a{display:inline-block;text-decoration:none;padding:15px 28px;margin:0 5px;border-radius:14px;border:2px solid #9a6a3c;font-size:1.1em;font-weight:700}
.nav a.on{background:#9a6a3c;color:#fff}.nav a.off{background:#fffdf8;color:#2b2620}
.divider{border-top:3px dashed #9a6a3c;margin:34px 0 4px}
.tophint{text-align:center;margin:16px 0}.tophint a{color:#9a6a3c;text-decoration:none;font-weight:700;font-size:.85em}
.xclose{position:fixed;top:10px;right:10px;z-index:99;background:#9a6a3c;color:#fff;width:56px;height:56px;line-height:52px;text-align:center;border-radius:50%;text-decoration:none;font-size:1.8em;font-weight:700;box-shadow:0 2px 6px rgba(0,0,0,.25)}
h1{text-align:center;font-size:1.7em;font-weight:700;margin-bottom:4px}
.sub{text-align:center;color:#7a7266;font-style:italic;margin-bottom:20px;font-size:.78em}
.card{background:#fffdf8;border:2px solid #e0d8c7;border-radius:18px;padding:20px 18px;margin-bottom:16px}
.card h2{font-size:1.1em;font-weight:700;margin-bottom:14px}
table.kpi{width:100%;border-collapse:separate;border-spacing:8px}
table.kpi td{width:50%;text-align:center;border:2px solid #e0d8c7;border-radius:14px;padding:13px 6px;vertical-align:middle}
.kv{font-size:1.85em;font-weight:700;color:#9a6a3c;line-height:1}.kl{font-size:.75em;color:#7a7266;margin-top:6px}
table.aw{width:100%;border-collapse:separate;border-spacing:8px}
table.aw td{width:33%;vertical-align:top;text-align:center;border:2px solid #e0d8c7;border-radius:14px;padding:16px 10px}
table.aw td.ea{border-color:#9a6a3c;background:#faf4ea}
table.aw td.lk{opacity:.45}
.bt{font-weight:700;font-size:.92em;line-height:1.2}.bd{color:#7a7266;font-size:.74em;margin-top:6px}
.lktx{font-size:.66em;margin-top:9px;color:#9a6a3c;font-weight:700;text-transform:uppercase;letter-spacing:.04em}
table.aw td.lk .lktx{color:#7a7266;font-weight:400}
.row{margin-bottom:14px}.row .rn{display:block;margin-bottom:5px}.row .rv{float:right;color:#7a7266}
.rn small{color:#7a7266;font-style:italic}
.track{height:15px;background:#e0d8c7;border-radius:8px}.fill{height:15px;border-radius:8px;background:#9a6a3c}
.book{font-size:1.15em;font-weight:700}.byau{color:#7a7266;font-style:italic;margin-top:2px}.bcnt{color:#7a7266;font-size:.72em;margin-top:5px}
.hl{margin-top:14px;padding:16px 20px;border-radius:0 12px 12px 0;line-height:1.55}
.note{margin-top:9px;padding:11px 15px;background:#fffdf8;border:2px dashed #e0d8c7;border-radius:12px;font-style:italic}
</style>'

# nav с JS-переключением. Тап теперь ДОХОДИТ до клика (пофикшен handleTapAtPoint в
# прошивке — сетевой guard больше не глотает клик), поэтому javascript:-ссылки работают.
# Выделения по умолчанию display:none → лёгкая стартовая страница (не виснет), грузятся
# по клику на вкладку. void(0) гасит навигацию, sw() переключает секции.
NAVHTML='<div class=nav><a id=tst class=on href="javascript:sw(1);void(0)">Статистика</a><a id=thl class=off href="javascript:sw(2);void(0)">Выделения</a></div>'

# ── награды ──
BADGES=""
ge(){ [ "${1%.*}" -ge "$2" ] 2>/dev/null && echo 1 || echo 0; }
badge(){ # title desc earned
  if [ "$3" = "1" ]; then cls=" class=ea"; lk="<div class=lktx>получено</div>"; else cls=" class=lk"; lk="<div class=lktx>цель</div>"; fi
  BADGES="$BADGES<td$cls><div class=bt>$1</div><div class=bd>$2</div>$lk</td>"
}
badge "Генерал от литературы" "$WAR книг про войны и историю" "$(ge "$WAR" 10)"
badge "Полная радуга" "выделял всеми цветами" "$(ge "$COLORS" 4)"
badge "Полиглот" "читаешь на $LANGS языках" "$(ge "$LANGS" 3)"
badge "Суток в книгах" "$HOURS ч ≈ $DAYS суток нон-стоп" "$(ge "$HOURS" 240)"
badge "Библиофаг" "$FIN книг дочитано" "$(ge "$FIN" 50)"
badge "Многостаночник" "$READING книг открыто разом" "$(ge "$READING" 40)"
badge "Гора Непрочитанного" "$UNRD книг ждут очереди" "$(ge "$UNRD" 40)"
badge "Сенчури" "дочитать 100 — сейчас $FIN" "$(ge "$FIN" 100)"
# разложить награды по 3 в ряд
AWROWS=$(printf '%s' "$BADGES" | sed 's#</td>#</td>\n#g' | awk 'NR%3==1{printf "<tr>"} {printf "%s",$0} NR%3==0{printf "</tr>"} END{if(NR%3)printf "</tr>"}')

# ── топ книг по времени (HTML прямо из SQL, с полосой) ──
BMAX=$(q "SELECT COALESCE(MAX(TimeSpentReading),1) FROM content WHERE $B AND TimeSpentReading>0")
TOPBOOKS=$(q "SELECT '<div class=row><span class=rv>'||ROUND(TimeSpentReading/3600.0,1)||' ч</span><span class=rn>'||
  replace(replace(replace(Title,'&','&amp;'),'<','&lt;'),'>','&gt;')||
  CASE WHEN Attribution IS NOT NULL AND Attribution<>'' THEN ' <small>'||replace(replace(Attribution,'<','&lt;'),'&','&amp;')||'</small>' ELSE '' END||
  '</span><div class=track><div class=fill style=\"width:'||MIN(100,ROUND(TimeSpentReading*100.0/$BMAX))||'%\"></div></div></div>'
  FROM content WHERE $B AND TimeSpentReading>0 ORDER BY TimeSpentReading DESC LIMIT 10")

# ── языки ──
LMAX=$(q "SELECT COALESCE(MAX(c),1) FROM (SELECT COUNT(*) c FROM content WHERE $B AND Language<>'' GROUP BY substr(Language,1,2))")
LANGROWS=$(q "SELECT '<div class=row><span class=rv>'||COUNT(*)||' кн.</span><span class=rn>'||
  CASE substr(Language,1,2) WHEN 'ru' THEN 'русский' WHEN 'en' THEN 'английский' WHEN 'fr' THEN 'французский' WHEN 'de' THEN 'немецкий' WHEN 'es' THEN 'испанский' ELSE substr(Language,1,2) END||
  '</span><div class=track><div class=fill style=\"width:'||ROUND(COUNT(*)*100.0/$LMAX)||'%\"></div></div></div>'
  FROM content WHERE $B AND Language<>'' GROUP BY substr(Language,1,2) ORDER BY COUNT(*) DESC LIMIT 6")

# ── выделения по цветам (полоски) ──
COLORBARS=""
for c in 0 3 1 2; do
  n=$(q "SELECT COUNT(*) FROM Bookmark WHERE $HLW AND COALESCE(Color,0)=$c")
  [ "${n:-0}" -gt 0 ] 2>/dev/null || continue
  case $c in 0) hx="#E6B800"; nm="жёлтый";; 1) hx="#D64545"; nm="красный";; 2) hx="#3B7DD8"; nm="синий";; 3) hx="#4C9A5A"; nm="зелёный";; esac
  w=$(q "SELECT ROUND($n*100.0/CASE WHEN $HL>0 THEN $HL ELSE 1 END)")
  COLORBARS="$COLORBARS<div class=row><span class=rv>$n</span><span class=rn><span style=\"display:inline-block;width:20px;height:20px;background:$hx;border-radius:5px;vertical-align:middle;margin-right:10px\"></span>$nm</span><div class=track><div class=fill style=\"width:${w}%;background:$hx\"></div></div></div>"
done

# ── выделения → в переменную (секция #sec-hl, БЕЗ отдельного файла = без навигации) ──
NB=$(q "SELECT COUNT(DISTINCT VolumeID) FROM Bookmark WHERE $HLW")
HLHTML=$(q "SELECT VolumeID FROM Bookmark WHERE $HLW GROUP BY VolumeID ORDER BY COUNT(*) DESC" | while IFS= read -r vol; do
  [ -n "$vol" ] || continue
  vq=$(printf '%s' "$vol" | sed "s/'/''/g")
  title=$(q "SELECT COALESCE(Title,'') FROM content WHERE ContentID='$vq' LIMIT 1"); [ -n "$title" ] || title="$vol"
  author=$(q "SELECT COALESCE(Attribution,'') FROM content WHERE ContentID='$vq' LIMIT 1")
  cnt=$(q "SELECT COUNT(*) FROM Bookmark WHERE $HLW AND VolumeID='$vq'")
  printf '<div class=card><div class=book>%s</div>' "$(esc "$title")"
  [ -n "$author" ] && printf '<div class=byau>%s</div>' "$(esc "$author")"
  printf '<div class=bcnt>%s выделений</div>' "$cnt"
  q "SELECT '<div class=hl style=\"background:'||
      CASE COALESCE(Color,0) WHEN 1 THEN '#f7dcdc;border-left:10px solid #D64545' WHEN 2 THEN '#d9e6f7;border-left:10px solid #3B7DD8' WHEN 3 THEN '#dcecdf;border-left:10px solid #4C9A5A' ELSE '#fbf3d0;border-left:10px solid #E6B800' END||'\">'||
      replace(replace(replace(TRIM(Text),'&','&amp;'),'<','&lt;'),'>','&gt;')||
      CASE WHEN Annotation IS NOT NULL AND TRIM(Annotation)<>'' THEN '<div class=note>Заметка: '||replace(replace(replace(TRIM(Annotation),'&','&amp;'),'<','&lt;'),'>','&gt;')||'</div>' ELSE '' END||
      '</div>' FROM Bookmark WHERE $HLW AND VolumeID='$vq' ORDER BY ChapterProgress"
  printf '</div>'
done)

# ── РЕНДЕР ОДНОЙ страницы index.html (обе секции, переключение на JS) ──
{
printf '<!doctype html><html lang=ru><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1"><title>Моя библиотека</title>%s</head><body><div class=wrap>' "$CSS"
printf '%s' "$NAVHTML"
printf '<div id=sec-stats>'
printf '<h1>Моя библиотека</h1><div class=sub>Kobo Libra Colour · обновлено %s · пересчитано только что</div>' "$NOW"
printf '<div class=card><table class=kpi><tr><td><div class=kv>%s</div><div class=kl>часов чтения</div></td><td><div class=kv>%s</div><div class=kl>книг в библиотеке</div></td></tr><tr><td><div class=kv>%s</div><div class=kl>дочитано</div></td><td><div class=kv>%s</div><div class=kl>выделений</div></td></tr><tr><td><div class=kv>%s</div><div class=kl>суток нон-стоп</div></td><td><div class=kv>%s</div><div class=kl>читаю сейчас</div></td></tr><tr><td><div class=kv>%s</div><div class=kl>авторов</div></td><td><div class=kv>%s</div><div class=kl>языка</div></td></tr></table></div>' "$HOURS" "$BOOKS" "$FIN" "$HL" "$DAYS" "$READING" "$AUTHORS" "$LANGS"
printf '<div class=card><h2>Награды</h2><table class=aw>%s</table></div>' "$AWROWS"
printf '<div class=card><h2>Топ книг по времени</h2>%s</div>' "$TOPBOOKS"
printf '<div class=card><h2>Выделения по цветам</h2>%s</div>' "$COLORBARS"
printf '<div class=card><h2>Языки</h2>%s</div>' "$LANGROWS"
printf '</div>'
printf '<div id=sec-hl style=display:none>'
printf '<h1>Мои выделения</h1><div class=sub>%s выделений из %s книг · обновлено %s</div>' "$HL" "$NB" "$NOW"
printf '%s' "$HLHTML"
printf '</div>'
printf '<script>function sw(t){var a=document.getElementById("sec-stats"),b=document.getElementById("sec-hl"),x=document.getElementById("tst"),y=document.getElementById("thl");a.style.display=t==1?"":"none";b.style.display=t==2?"":"none";x.className=t==1?"on":"off";y.className=t==2?"on":"off";window.scrollTo(0,0);}</script>'
printf '</div></body></html>'
} > "$BASE/index.html"
rm -f "$BASE/stats.html" "$BASE/highlights.html" 2>/dev/null   # старые файлы больше не нужны

echo "koboviz-gen: $HOURS ч, $FIN дочитано, $HL выделений → $BASE/index.html ($NOW)"
