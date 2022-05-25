
template <typename... Args> class Tuple;

template <> class Tuple<> { };

template <typename T, typename... Args> 
class Tuple<T, Args...> : Tuple<Args...> 
{

};

int main()
{

}