#!/bin/bash

HOST="localhost"
PORT="3490"
TOTAL=${1:-100}   # number of requests (pass as first arg, default 100)
CONCURRENCY=${2:-50}  # simultaneous connections (pass as second arg, default 50)

success=0
fail=0
refused=0

echo "Stress testing $HOST:$PORT — $TOTAL requests, $CONCURRENCY at a time"
echo "------------------------------------------------------------"

run_request() {
    response=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$HOST:$PORT/" 2>&1)
    exit_code=$?
    if [ $exit_code -eq 7 ]; then
        echo "REFUSED"
    elif [ $exit_code -ne 0 ] || [ "$response" = "000" ]; then
        echo "FAIL:$exit_code"
    else
        echo "OK:$response"
    fi
}

export -f run_request
export HOST PORT

results=$(seq 1 $TOTAL | xargs -P $CONCURRENCY -I{} bash -c 'run_request')

while IFS= read -r line; do
    case "$line" in
        OK:*)    ((success++)) ;;
        REFUSED) ((refused++)) ;;
        FAIL:*)  ((fail++)) ;;
    esac
done <<< "$results"

echo ""
echo "Results:"
echo "  Success:  $success"
echo "  Failed:   $fail"
echo "  Refused:  $refused"
echo "  Total:    $TOTAL"
echo ""

if [ $refused -gt 0 ]; then
    echo "Server refused connections — backlog (BACKLOG=10) or fork limit hit."
fi
if [ $fail -gt 0 ]; then
    echo "Some requests failed — likely recv buffer overflow (MAXDATASIZE=100) or process limit."
fi
if [ $success -eq $TOTAL ]; then
    echo "All requests succeeded. Try a higher concurrency: ./stress_test.sh $TOTAL $((CONCURRENCY * 2))"
fi
