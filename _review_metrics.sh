#!/usr/bin/env bash
set -e
TARGET="E:/DouBao2/Project/KatCode/Project02/src"
cd "E:/DouBao2/Project/KatCode/Project02"

# Collect files
mapfile -t FILES < <(find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) | sort)
echo "=== 总览 ==="
echo "源文件数: ${#FILES[@]}"
echo ""

TOTAL_LINES=0
for f in "${FILES[@]}"; do
  L=$(wc -l < "$f" 2>/dev/null || echo 0)
  TOTAL_LINES=$((TOTAL_LINES + L))
done
echo "总代码行数: $TOTAL_LINES"
echo ""

echo "=== 最大的 15 个文件 ==="
for f in "${FILES[@]}"; do
  L=$(wc -l < "$f" 2>/dev/null || echo 0)
  printf "%6d  %s\n" "$L" "$f"
done | sort -rn | head -15
echo ""

echo "=== 注释率（注释行 / 代码行）==="
for f in "${FILES[@]}"; do
  total=$(wc -l < "$f" 2>/dev/null || echo 0)
  blanks=$(grep -cE "^[[:space:]]*$" "$f" 2>/dev/null | head -1)
  comments=$(grep -cE "^\s*(#|//|/\*)" "$f" 2>/dev/null | head -1)
  blanks=${blanks:-0}
  comments=${comments:-0}
  code=$((total - blanks - comments))
  code=${code:-0}
  if [ "$code" -gt 0 ]; then
    pct=$((comments * 100 / code))
  else
    pct=0
  fi
  printf "%3d%%  (代码 %4d / 注释 %4d)  %s\n" "$pct" "$code" "$comments" "$f"
done | sort -rn | head -10
echo ""
echo "=== 注释率最低的 10 个文件 ==="
for f in "${FILES[@]}"; do
  total=$(wc -l < "$f" 2>/dev/null || echo 0)
  blanks=$(grep -cE "^[[:space:]]*$" "$f" 2>/dev/null | head -1)
  comments=$(grep -cE "^\s*(#|//|/\*)" "$f" 2>/dev/null | head -1)
  blanks=${blanks:-0}
  comments=${comments:-0}
  code=$((total - blanks - comments))
  code=${code:-0}
  if [ "$code" -gt 0 ]; then
    pct=$((comments * 100 / code))
  else
    pct=0
  fi
  printf "%3d%%  %s\n" "$pct" "$f"
done | sort -n | head -10
echo ""

echo "=== 单文件内圈复杂度粗略估算 (branch count / function count) ==="
for f in "${FILES[@]}"; do
  total=$(wc -l < "$f" 2>/dev/null || echo 0)
  branches=$(grep -cE "\b(if|else|for|while|case|catch|switch|&&|\|\|)\b" "$f" 2>/dev/null | head -1)
  funcs=$(grep -cE "^[a-zA-Z_:&<>\* ]+\s+[a-zA-Z_:]+\s*\(" "$f" 2>/dev/null | head -1)
  branches=${branches:-0}
  funcs=${funcs:-0}
  if [ "$funcs" -gt 3 ]; then
    cc=$((branches / funcs))
  else
    cc=0
  fi
  printf "  CC~%2d  funcs=%2d  branches=%3d  %s\n" "$cc" "$funcs" "$branches" "$f"
done | sort -rn | head -10
echo ""

echo "=== TODO/FIXME/HACK/XXX 标记 ==="
for f in "${FILES[@]}"; do
  matches=$(grep -nEi "TODO|FIXME|HACK|XXX" "$f" 2>/dev/null | head -3)
  if [ -n "$matches" ]; then
    echo "--- $f ---"
    echo "$matches"
  fi
done
echo ""

echo "=== 调试输出语句 (printf/cout 没用) ==="
for f in "${FILES[@]}"; do
  matches=$(grep -nE "fprintf\(stderr|std::cout" "$f" 2>/dev/null)
  if [ -n "$matches" ]; then
    echo "--- $f ---"
    echo "$matches" | head -5
  fi
done
