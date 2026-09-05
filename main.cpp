#include <winnow/error.hpp>
#include <winnow/utils.hpp>
#include <winnow/result.hpp>


template <typename O, typename E>
using ModalResult = winnow::modal_result<O, winnow::err_mode<E>>;

int main()
{

}
