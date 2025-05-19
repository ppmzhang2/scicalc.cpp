# Scientific Calculator

## Pratt Parser

Parsing and Evaluating:

```
3! - ln(5-1) + 3^2 / 7
```

### Split the Expression String

- Input: `char* str`

```none
"3! - ln(5-1) + 3^2 / 7"
```

- Output: `std::vector<char *>`.
  Each element is a token string (either a number or a symbolic operator).

```
[
  "3", "!", "-", "ln", "(", "5", "-", "1", ")", "+", "3", "^", "2", "/", "7"
]
```

### Encoding

Each `char *` substring is converted into an `Atom` structure:

- a boolean flag indicating if the token is an operator.
- a float value representing either the number or the `enum` value of the
  operator.

| Token | Op Flag | Value | Enum        |
| ----- | ------- | ----- | ----------- |
| "3"   | false   | 3.0   |             |
| "!"   | true    | 101   | `Sign::FCT` |
| "-"   | true    | 103   | `Sign::SUB` |
| "ln"  | true    | 102   | `Sign::LOG` |
| "("   | true    | 1     | `Sign::PAL` |
| "5"   | false   | 5.0   |             |
| "-"   | true    | 103   | `Sign::SUB` |
| "1"   | false   | 1.0   |             |
| ")"   | true    | 2     | `Sign::PAR` |
| "+"   | true    | 104   | `Sign::ADD` |
| "3"   | false   | 3.0   |             |
| "^"   | true    | 106   | `Sign::EXP` |
| "2"   | false   | 2.0   |             |
| "/"   | true    | 105   | `Sign::DIV` |
| "7"   | false   | 7.0   |             |

### Tokenization

This process services two main purposes:

- **Assigning Binding Powers (BP)**:
  the BP is decoupled from operators and parentheses will be removed before
  elavating BPs of operators enclosed.
  Note that a unary operator (like `!`) should always have the BP value of one
  side equal to 0.

- **Evaluating Constants**:
  constant symbols are evaluated to their respective values.

The tokens are in reverse order as the following chaining process should start
from the rightmost token.

| Type | Value / (IDX, LBP RBP) |
| ---- | ---------------------- |
| Num  | 7.0                    |
| DIV  | (105, 2, 2)            |
| Num  | 2.0                    |
| EXP  | (106, 4, 3)            |
| Num  | 3.0                    |
| ADD  | (104, 1, 1)            |
| Num  | 1.0                    |
| SUB  | (103, 11, 11)          |
| Num  | 5.0                    |
| LOG  | (102, 0, 5)            |
| SUB  | (103, 1, 1)            |
| FCT  | (101, 6, 0)            |
| Num  | 3.0                    |

### Chaining

Constructing from tokens a **linked list of `Chain` nodes**, using operator
precedence and associativity rules.
Each `Chain` node contains one operator (including its index, LBP, RBP) and
potentially its LHS operand `num`.
Its potential RHS operand is in the next node pointed by `next`.
The only exception is the last non-null node which should be an isolated
operand i.e. either a number or a number with a unary operator.

This is one of the core features of parser as it need to correctly associate
each operator with its operands and the rest will be merely a matter of
prioritization by comparing LBP and RBP.

| No. | LHS | Operator      | LBP | RBP |
| --- | --- | ------------- | --- | --- |
| 1   | 3.0 | `FCT` (left)  | 6   | 0   |
| 2   |     | `SUB` (infix) | 1   | 1   |
| 3   |     | `LOG` (right) | 0   | 5   |
| 3   | 5.0 | `SUB` (infix) | 0   | 5   |
| 4   | 1.0 | `ADD` (infix) | 11  | 11  |
| 5   | 3.0 | `EXP` (infix) | 1   | 1   |
| 6   | 2.0 | `DIV` (infix) | 4   | 3   |
| 7   | 7.0 |               | 2   | 2   |

Except for the trivial case of infix operators, two main rules on nodes with
unary operators are:

- a node with left-associativity unary operator should always have a LHS, and
  its RHS i.e. the LHS of the next node should be null.
- a node with right-associativity unary operator should always have a null LHS,
  yet its RHS i.e. the LHS of the next node should not be null.

### Reduction

This function recursively collapses the chain by evaluating either the current
`Chain` node or defer to the next, depending on the operator's associativity
and the binding power:

- For left-associative unary operators, evaluate if its `lbp > 0`
- For right-associative unary operators or infix operators, evaluate if
  `rbp > next->lbp`
- Deferring to the next is also needed if any operand required is not
  available.

Steps:

1. 3! = 6
2. 5 - 1 = 4
3. ln(4) = 1.386
4. 6 - 1.386 = 4.614
5. 3^2 = 9
6. 9 / 7 = 1.286
7. 4.614 + 1.286 = 5.9
