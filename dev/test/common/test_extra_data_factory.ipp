namespace test
{

struct ud_factory_t
{
	struct data_t
	{
		int m_a;
		int m_b;
		int m_c;
		void * m_d;
	};

	void make_within( restinio::extra_data_buffer_t<data_t> buf ) noexcept
	{
		new(buf.get()) data_t{ 0, 1, 2, buf.get() };
	}
};

} /* namespace test */

