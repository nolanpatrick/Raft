# North

My own take on an interpreted Forth-like language, inspired by Alexey Kutepov (Tsoding)'s [Porth](https://gitlab.com/tsoding/porth).

## Compiling

To compile, you should only need to run the build script. At this time, I have only included a Windows script, 
but once the project is further into development a *nix version will be added.

## Project Milestones
Coming soon

## Commands & Operations

### Arithmetic

| Command | Action         | Arguments            | Output           |
|---------|----------------|----------------------|------------------|
| ```+``` | Addition       | ```a: int, b: int``` | ```a + b: int``` |
| ```-``` | Subtraction    | ```a: int, b: int``` | ```a - b: int``` |
| ```*``` | Multiplication | ```a: int, b: int``` | ```a * b: int``` |
| ```/``` | Int Division   | ```a: int, b: int``` | ```a / b: int``` |
| ```%``` | Modulus        | ```a: int, b: int``` | ```a % b: int``` |

### Boolean
Boolean values are represented as integers either ```0: false``` or ```1: true```. 

| Command    | Action       | Arguments              | Output              |
|------------|--------------|------------------------|---------------------|
| ```and```  | And          | ```a: bool, b: bool``` | ```a and b: bool``` |
| ```or```   | Or           | ```a: bool, b: bool``` | ```a or b: bool```  |
| ```not```  | Invert       | ```a: bool```          | ```not a: bool```   |
| ```<```    | Less than    | ```a: int, b: int```   | ```a < b: bool```   |
| ```>```    | Greater than | ```a: int, b: int```   | ```a > b: bool```   |
| ```==```   | Equal to     | ```a: int, b: int```   | ```a == b: bool```  |