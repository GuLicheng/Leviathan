#include <llvm/ADT/Any.h>

#include <llvm/Support/JSON.h>

int main()
{
    llvm::Any a = 4;
    a = 5;

}