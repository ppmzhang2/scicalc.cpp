# Scientific Calculator

# Pratt Parser (V1)

| Step | min_bp | Token In | Created LHS                           | Peeked   | Action Taken                    |
| ---- | ------ | -------- | ------------------------------------- | -------- | ------------------------------- |
| 1    | 0      | 2 (NUM)  | Expr(2)                               | + (1,1)  | 1 > 0 → recurse with min_bp = 1 |
| 2    | 1      | 3 (NUM)  | Expr(3)                               | - (1,1)  | 1 <= 1 → stop recursion         |
| 1'   | 0      | +        | Expr(+, 2, 3)                         | - (1,1)  | 1 > 0 → recurse with min_bp = 1 |
| 3    | 1      | 6 (NUM)  | Expr(6)                               | ^ (4,3)  | 4 > 1 → recurse with min_bp = 3 |
| 4    | 3      | 2 (NUM)  | Expr(2)                               | + (1,1)  | 1 <= 3 → stop recursion         |
| 3'   | 1      | ^        | Expr(^, 6, 2)                         | + (1,1)  | 1 <= 1 → stop recursion         |
| 1''  | 0      | -        | Expr(-, Expr(+, 2, 3), Expr(^, 6, 2)) | + (1,1)  | 1 > 0 → recurse with min_bp = 1 |
| 5    | 1      | 4 (NUM)  | Expr(4)                               | \* (2,2) | 2 > 1 → recurse with min_bp = 2 |
| 6    | 2      | 5 (NUM)  | Expr(5)                               | (NONE)   | End of input                    |
| 5'   | 1      | \*       | Expr(\*, 4, 5)                        | (NONE)   | End of input                    |
| 1''' | 0      | +        | Expr(+, ..., Expr(\*, 4, 5))          | (NONE)   | Final AST completed             |
