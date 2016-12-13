#!/bin/bash

# Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Usage
if [ "$#" -lt 3 ]
then
  echo "$0: Benchmark memory usage of IoT.js"
  echo ""
  echo "Usage: $0 [-d] IOTJS IOTJS_MEMSTATS BENCHMARK..."
  echo ""
  echo "Positional arguments:"
  echo "  IOTJS             path to IoT.js engine built without memory"
  echo "                    statistics support"
  echo "  IOTJS_MEMSTATS    path to IoT.js engine built with memory statistics"
  echo "                    support"
  echo "  BENCHMARK...      paths to JavaScript programs to be used as the"
  echo "                    benchmark suite"
  echo ""
  echo "Optional arguments:"
  echo "  -d                generate semicolon-delimited output (default:"
  echo "                    formatted human-readable output)"
  echo ""
  echo "The tool benchmarks the memory usage of IoT.js with the help of two"
  echo "different builds and a suite of JavaScript programs. Each benchmark"
  echo "script is executed by both builds: the \"memstats\" build reports"
  echo "statistics retrieved from JerryScript, while the \"normal\" build"
  echo "reports RSS results."
  exit 1
fi

# Choosing table or semicolon-separated output mode
if [ "$1" == "-d" ]
then
  TABLE="no"
  PRINT_TEST_NAME_AWK_SCRIPT='{printf "%s;", $1}'
  PRINT_TOTAL_AWK_SCRIPT='{printf "%d;%d\n", $1, $2 * 1024}'

  shift
else
  PRINT_TEST_NAME_AWK_SCRIPT='{printf "%30s", $1}'
  PRINT_TOTAL_AWK_SCRIPT='{printf "%25d%25d\n", $1, $2 * 1024}'
  TABLE="yes"
fi

function fail_msg
{
  echo "$1"
  exit 1
}

# Engine

# Check if the specified build supports memory statistics options
function is_mem_stats_build
{
  [ -x "$1" ] || fail_msg "Engine '$1' is not executable"

  tmpfile=`mktemp`
  "$1" $tmpfile --memstat 2>&1 | \
    grep -- "Ignoring memory statistics option" 2>&1 > /dev/null
  code=$?
  rm $tmpfile

  return $code
}

IOTJS=$(readlink -f "$1")
shift
is_mem_stats_build "$IOTJS" || fail_msg \
  "First engine specified should be built without memory statistics support"

IOTJS_MEM_STATS=$(readlink -f "$1")
shift
is_mem_stats_build "$IOTJS_MEM_STATS" && fail_msg \
  "Second engine specified should be built with memory statistics support"

# Benchmarks list
BENCHMARKS=""

while [ $# -ne 0 ]
do
  BENCHMARKS="$BENCHMARKS $1"
  shift
done

# Running
if [ "$TABLE" == "yes" ]
then
  awk 'BEGIN {printf "%30s%25s%25s\n", "Test name", "Peak Heap (jerry)", \
    "Maximum RSS"}'
  echo
fi

STARTDIR=$(pwd)

for bench in $BENCHMARKS
do
  bench_name=$(basename -s '.js' $bench)
  bench_canon=$(readlink -f $bench)

  cd `dirname $bench_canon`

  echo "$bench_name" | awk "$PRINT_TEST_NAME_AWK_SCRIPT"
  MEM_STATS=$("$IOTJS_MEM_STATS" $bench_canon --memstat | \
    grep -e "Peak allocated =" | grep -o "[0-9]*")
  RSS=$($STARTDIR/deps/jerry/tools/rss-measure.sh "$IOTJS" $bench_canon | \
    tail -n 1 | grep -o "[0-9]*")
  echo $MEM_STATS $RSS | xargs | awk "$PRINT_TOTAL_AWK_SCRIPT"

  cd $STARTDIR
done
