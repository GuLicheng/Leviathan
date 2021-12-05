
struct B
{
    B() = default;
    B(const B&) = default;
    // B(B&&) = default;
    B& operator=(const B&) = default;
};


struct D : public B
{
    using B::B;
    using B::operator=;
};

int main()
{
    D d;
    B b;
    d = b;
    D dd = (D&&)b;
}