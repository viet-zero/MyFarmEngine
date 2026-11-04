# Real CLI must fail with a useful cross-reference diagnostic, not merely parse args.
string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef nonce)
execute_process(COMMAND "${AUDIT}" --coverage "${BUILD}/missing_coverage_${nonce}.csv"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 30)
if(NOT status EQUAL 1 OR NOT errors MATCHES "coverage_manifest_missing")
    message(FATAL_ERROR "Missing coverage not rejected by AssetAudit: ${status}\n${output}\n${errors}")
endif()
message(STATUS "AssetAudit exits 1 with coverage_manifest_missing")
