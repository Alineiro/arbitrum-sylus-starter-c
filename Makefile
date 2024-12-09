STACK_SIZE=1024
CC=clang
LD=wasm-ld
CFLAGS=-Iinclude/ -Istylus-sdk-c/include --target=wasm32 -Os --no-standard-libraries -mbulk-memory -Wall -g -Wno-incompatible-function-pointer-types
LDFLAGS=-O2 --no-entry --stack-first -z stack-size=$(STACK_SIZE)

NAME = contract.wasm
C_FILES = $(wildcard contracts/*.c) $(wildcard stylus-sdk-c/src/*.c)
OBJECTS = $(patsubst %.c, %.o, $(C_FILES))

all: $(NAME)

# Step 1: turn C files into Object files
$(OBJECTS): %.o: %.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Step 2: link
contract_unstripped.wasm: $(OBJECTS)
	@$(LD) $(OBJECTS) $(LDFLAGS) -o $@

# Step 3: strip symbols from wasm
$(NAME): contract_unstripped.wasm
	@wasm-strip contract_unstripped.wasm
	cp contract_unstripped.wasm contract.wasm

# Step 4: check the wasm using cargo-stylus
# cargo stylus check --wasm-file ./contract.wasm -e https://sepolia-rollup.arbitrum.io/rpc

# Step 5: deploy the wasm using cargo-stylus
# cargo stylus deploy --wasm-file ./contract.wasm -e https://sepolia-rollup.arbitrum.io/rpc --cargo-stylus-version 0.5.3 --private-key PRIVATE_KEY 

clean:
	@rm $(OBJECTS) contract_unstripped.wasm contract.wasm

re: clean all

play: all
	@npm run play

frontend: all
	@npm run frontend

.phony: all clean re play frontend
