# Lua Lexer
A Lua lexer written in C.

## Performance
The lexer is currently parsing the input stream byte for byte which means that it is not as fast as it could be if it were written as a `Deterministic State Machine`.

That is fine for now, but I would like to change that some time in the near future. 
Though there is probably something else that is making the program a lot slower than it should be, because parsing a single digit repeated a million times should not take longer than parsing nine digits a million times.

The current performance on an `AMD Ryzen 7 3700X` is as follows:

### Identifiers
| N | Input Data | Time |
| --- | --- | --- |
| 100K | id | 0.6s |
| 1M | id | 10.0s |
| 100K | identifiers | 2.1s |
| 1M | identifiers | 18.9s |

### Numbers
| N | Input Data | Time |
| --- | --- | --- |
| 100K | 1 | 0.3s |
| 1M | 1 | 43s |
| 100K | 123456789 | 2s |
| 1M | 123456789 | ? |

### Strings
| N | Input Data | Time |
| --- | --- | --- |
| 100K | "a" | 1.4s |
| 1M | "a" | 108s |
| 100K | "abcdefghijklmnopqrstuvwxyz" | 92s |
| 1M | "abcdefghijklmnopqrstuvwxyz" | ? |


where:
- `N` is how many times the input data was repeated
- `Input Data` is the task being run
- `Time` is how long it took in seconds to complete that task

## References
[Lexical Analysis](https://en.wikipedia.org/wiki/Lexical_analysis)

[Strategies For Fast Lexical Analysis when Parsing Programming Languages](https://www.nothings.org/computer/lexing.html)
