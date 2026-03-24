#!/bin/bash
# Test 9.7: Test export/import cycle (metadata preservation, duplicate handling)

set -e

echo "=== Testing Export/Import Cycle ==="

# Create a test exercise
TEST_EXERCISE_DIR="subjects/generated/test_rank/test_export_import"
TEST_EXERCISE_NAME="test_export_import"

# Create exercise structure
mkdir -p "$TEST_EXERCISE_DIR/attachment"
mkdir -p "$TEST_EXERCISE_DIR/.validation"

# Create exercise files
cat > "$TEST_EXERCISE_DIR/metadata.json" << EOF
{
  "generated_at": "2026-03-24T12:00:00Z",
  "prompt": "Test exercise for export/import",
  "rank": "test_rank",
  "level": "level0",
  "type": "function",
  "model": "test-model",
  "quality_score": 85,
  "validation_rounds": 2
}
EOF

cat > "$TEST_EXERCISE_DIR/attachment/subject.en.txt" << EOF
# Test Exercise

This is a test exercise for export/import functionality.
EOF

cat > "$TEST_EXERCISE_DIR/solution.c" << EOF
#include <stdio.h>

int main() {
    printf("42\\n");
    return 0;
}
EOF

cat > "$TEST_EXERCISE_DIR/tester.sh" << EOF
#!/bin/bash
echo "42"
EOF

chmod +x "$TEST_EXERCISE_DIR/tester.sh"

cat > "$TEST_EXERCISE_DIR/.validation/quality_report.json" << EOF
{
  "overall_score": 85,
  "approved": true
}
EOF

echo "✓ Created test exercise at $TEST_EXERCISE_DIR"

# Test export
echo ""
echo "Testing export..."
./examcli share "$TEST_EXERCISE_NAME" --rank test_rank -o "/tmp/test_export_import.tar.gz"

if [ -f "/tmp/test_export_import.tar.gz" ]; then
    echo "✓ Export successful: /tmp/test_export_import.tar.gz"

    # Verify archive contents
    echo ""
    echo "Archive contents:"
    tar -tzf "/tmp/test_export_import.tar.gz" | head -10

    # Test import
    echo ""
    echo "Testing import..."
    ./examcli import "/tmp/test_export_import.tar.gz" --rank test_rank_import

    # Check if imported correctly
    IMPORTED_DIR="subjects/generated/test_rank_import/test_export_import"
    if [ -d "$IMPORTED_DIR" ]; then
        echo "✓ Import successful: $IMPORTED_DIR"

        # Verify metadata preservation
        if [ -f "$IMPORTED_DIR/metadata.json" ]; then
            echo "✓ Metadata preserved"

            # Check for imported_at field
            if grep -q "imported_at" "$IMPORTED_DIR/metadata.json"; then
                echo "✓ Imported timestamp added"
            else
                echo "⚠ imported_at field not found"
            fi

            # Check original fields preserved
            if grep -q "test-model" "$IMPORTED_DIR/metadata.json"; then
                echo "✓ Original model preserved"
            fi

            if grep -q '"quality_score": 85' "$IMPORTED_DIR/metadata.json"; then
                echo "✓ Quality score preserved"
            fi
        else
            echo "✗ Metadata not preserved"
        fi

        # Test duplicate handling
        echo ""
        echo "Testing duplicate handling..."
        ./examcli import "/tmp/test_export_import.tar.gz" --rank test_rank_import

        # Check if timestamp was appended
        DUPLICATE_DIR=$(ls -d subjects/generated/test_rank_import/test_export_import_* 2>/dev/null | tail -1)
        if [ -n "$DUPLICATE_DIR" ] && [ "$DUPLICATE_DIR" != "$IMPORTED_DIR" ]; then
            echo "✓ Duplicate name handled with timestamp"
        else
            echo "⚠ Duplicate handling may need review"
        fi

    else
        echo "✗ Import failed"
        exit 1
    fi

else
    echo "✗ Export failed"
    exit 1
fi

# Cleanup
echo ""
echo "Cleaning up..."
rm -f "/tmp/test_export_import.tar.gz"
rm -rf "subjects/generated/test_rank"
rm -rf "subjects/generated/test_rank_import"

echo ""
echo "=== Export/Import Tests Complete ==="
