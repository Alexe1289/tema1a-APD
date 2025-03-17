#!/bin/bash

# Initialize variables
script="./run_with_docker.sh" # Replace with your script's path
count_not_84=0 # Counter for non-84/84 results
iterations=10 # Number of times to run the script

rm fisier
# Loop to run the script 10 times
for ((i=1; i<=iterations; i++)); do
    echo "Running iteration $i..."
    output=$($script) # Run the script and capture the output
    echo "$output" >> fisier # Print the output for debugging (optional)
    
    # Check if the output contains "Total: 84/84"
    if ! echo "$output" | grep -q "Total:       84/84"; then
        count_not_84=$((count_not_84 + 1)) # Increment the counter if it doesn't match
    fi
done

# Display the result
echo "Number of times the result was NOT 84/84: $count_not_84"
