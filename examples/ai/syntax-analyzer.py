# ==========================================
# 1. TOKENS & LEXER (Lexical Analysis)
# ==========================================
# We need to turn a string like "3 + 5" into tokens: [INT(3), PLUS, INT(5)]


class Token:
    def __init__(self, type_, value=None):
        self.type = type_
        self.value = value

    def __repr__(self):
        return f"Token({self.type}, {repr(self.value)})"


class Lexer:
    def __init__(self, text):
        self.text = text
        self.pos = 0
        self.current_char = self.text[self.pos] if text else None

    def advance(self):
        self.pos += 1
        self.current_char = self.text[self.pos] if self.pos < len(self.text) else None
        print(self.current_char);

    def skip_whitespace(self):
        while self.current_char is not None and self.current_char.isspace():
            self.advance()

    def get_integer(self):
        result = ''
        while self.current_char is not None and self.current_char.isdigit():
            result += self.current_char
            self.advance()
        return int(result)

    def get_next_token(self):
        while self.current_char is not None:
            if self.current_char.isspace():
                self.skip_whitespace()
                continue
            if self.current_char.isdigit():
                return Token('INT', self.get_integer())
            if self.current_char == '+':
                self.advance()
                return Token('PLUS', '+')
            if self.current_char == '-':
                self.advance()
                return Token('MINUS', '-')
            if self.current_char == '*':
                self.advance()
                return Token('MUL', '*')
            if self.current_char == '/':
                self.advance()
                return Token('DIV', '/')
            if self.current_char == '(':
                self.advance()
                return Token('LPAREN', '(')
            if self.current_char == ')':
                self.advance()
                return Token('RPAREN', ')')

            raise Exception(f"Lexer Error: Invalid character '{self.current_char}'")
        return Token('EOF')  # End of file/string

# ==========================================
# 2. ABSTRACT SYNTAX TREE (AST) NODES
# ==========================================
# These classes represent the nodes in our tree.


class NumberNode:
    def __init__(self, token):
        self.token = token
        self.value = token.value

    def __repr__(self):
        return f"Num({self.value})"


class BinOpNode:
    def __init__(self, left, op, right):
        self.left = left
        self.token = self.op = op
        self.right = right

    def __repr__(self):
        return f"({self.left} {self.op.value} {self.right})"

# ==========================================
# 3. SYNTAX ANALYZER (The Parser)
# ==========================================


class Parser:
    def __init__(self, lexer):
        self.lexer = lexer
        self.current_token = self.lexer.get_next_token()

    def eat(self, token_type):
        # Compare current token type with the expected one.
        # If they match, "eat" it and move to the next token.
        if self.current_token.type == token_type:
            self.current_token = self.lexer.get_next_token()
            print(self.current_token)
        else:
            raise Exception(f"Syntax Error: Expected {
                            token_type}, got {self.current_token.type}")

    def factor(self):
        """factor -> INT | LPAREN expr RPAREN"""
        token = self.current_token

        if token.type == 'INT':
            self.eat('INT')
            return NumberNode(token)
        elif token.type == 'LPAREN':
            self.eat('LPAREN')
            node = self.expr()  # Recursively parse the expression inside
            self.eat('RPAREN')
            return node

    def term(self):
        """term -> factor ((MUL | DIV) factor)*"""
        node = self.factor()

        while self.current_token.type in ('MUL', 'DIV'):
            token = self.current_token
            if token.type == 'MUL':
                self.eat('MUL')
            elif token.type == 'DIV':
                self.eat('DIV')
            # Create a Binary Operation node branching to the next factor
            node = BinOpNode(left=node, op=token, right=self.factor())

        return node

    def expr(self):
        """expr -> term ((PLUS | MINUS) term)*"""
        node = self.term()

        while self.current_token.type in ('PLUS', 'MINUS'):
            token = self.current_token
            if token.type == 'PLUS':
                self.eat('PLUS')
            elif token.type == 'MINUS':
                self.eat('MINUS')
            # Create a Binary Operation node branching to the next term
            node = BinOpNode(left=node, op=token, right=self.term())

        return node

    def parse(self):
        return self.expr()


# ==========================================
# 4. RUNNING IT
# ==========================================
if __name__ == '__main__':
    text = "3 + 5 * (10 - 2) + 2 * 3 + 1"
    print(f"Source Code: {text}\n")

    lexer = Lexer(text)
    parser = Parser(lexer)

    ast = parser.parse()
    print("Generated AST:")
    print(ast)
