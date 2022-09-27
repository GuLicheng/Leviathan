
struct Base
{
	virtual Base* self() { return this; }
};


struct Derived : Base
{
	virtual Derived* self() override { return this; }
};


int main()
{
	auto b = new Derived();
	b->self();
}