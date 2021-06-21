
|type  |           function |      O0  |        O3  |     desc |
|------|--------------------|:--------:|:----------:|----------|
|shl1b | checksum_add       | 619.80 ns|   22.97 ns | 直接累加 |
|shl1b | checksum_sse       | 161.77 ns|   29.68 ns | SSE      |
|shl1b | checksum_sse_4loop | 122.61 ns|   24.12 ns | 4路SSE   |

