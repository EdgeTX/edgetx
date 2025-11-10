#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Define paths
SRC_LIST="$SCRIPT_DIR/convert-gfx-list.csv"
SRC_DIR="$SCRIPT_DIR/../radio/src/bitmaps/img-src"
TEMP_FILE="$SCRIPT_DIR/convert-gfx-list.csv.tmp"

# Check if CSV file exists
if [ ! -f "$SRC_LIST" ]; then
    echo "Error: CSV file not found: $SRC_LIST"
    exit 1
fi

# Check if SVG source directory exists
if [ ! -d "$SRC_DIR" ]; then
    echo "Error: SVG source directory not found: $SRC_DIR"
    exit 1
fi

echo "Updating CSV with SVG modification dates..."

# Process the CSV file
line_num=0
while IFS=';' read -r file width height rest || [ -n "$file" ]; do
    line_num=$((line_num + 1))
    
    # Handle header line
    if [ $line_num -eq 1 ]; then
        # Update header to include date column if not present
        if [[ "$file" == "file" ]]; then
            echo "file;width;height;modified" >> "$TEMP_FILE"
        else
            echo "$file;$width;$height;modified" >> "$TEMP_FILE"
        fi
        continue
    fi
    
    # Skip empty lines
    if [ -z "$file" ]; then
        echo "" >> "$TEMP_FILE"
        continue
    fi
    
    # Get SVG file modification date
    svg_path="$SRC_DIR/$file.svg"
    if [ -f "$svg_path" ]; then
        # Get modification date in ISO 8601 format (YYYY-MM-DD HH:MM:SS)
        # macOS stat format
        mod_date=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$svg_path" 2>/dev/null)
        
        # Fallback for Linux systems (stat has different syntax)
        if [ -z "$mod_date" ]; then
            mod_date=$(stat -c "%y" "$svg_path" 2>/dev/null | cut -d'.' -f1)
        fi
        
        echo "$file;$width;$height;$mod_date" >> "$TEMP_FILE"
    else
        # SVG file not found, leave date empty
        echo "$file;$width;$height;" >> "$TEMP_FILE"
    fi
done < "$SRC_LIST"

# Replace original file with updated one
mv "$TEMP_FILE" "$SRC_LIST"

echo "Done! CSV updated with modification dates."
echo "Updated file: $SRC_LIST"
