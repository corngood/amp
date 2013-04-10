//#include <array>
#include <iostream>
#include <initializer_list>

#if __CXXAMP__
#define __AMP_global __attribute__((address_space(0xFFFF00)))
#else
#define __AMP_global
#endif

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL/cl.hpp>

namespace concurrency
{
	template <int Rank> class extent
	{
		int value[Rank];

	public:
		int &operator[](int const i) { return value[i]; }
		int const &operator[](int const i) const { return value[i]; }

		extent() {}
		extent(std::initializer_list<int> list)
		{
			auto e = list.begin();
			for(int i = 0; i < Rank; ++i) value[i] = *e++;
		}
	};

	template <int Rank> class index : public extent<Rank>
	{
	public:
		index() {}
		index(std::initializer_list<int> list) : extent<Rank>(list) {}
	};

	template <typename ValueType, int Rank = 1> class array_view
	{
		extent<Rank> extent;
		ValueType __AMP_global * data;
		cl::Buffer buffer;

		int getIndex(index<Rank> const &index) const
		{
			int stride = 1;
			int p = 0;
			for(int i = Rank - 1; i >= 0; --i)
			{
				p = index[i] * stride + p;
				stride = stride * extent[i];
			}
			return p;
		}

	public:
		array_view(int size0, int size1, ValueType *data) :
			data((ValueType __AMP_global *)data)
		{
			extent[0] = size0;
			extent[1] = size1;
		}

		concurrency::extent<Rank> get_extent() const
		{
			return extent;
		}

		ValueType __AMP_global &operator[](index<Rank> const &index) const
		{
			return data[getIndex(index)];
		}

		ValueType __AMP_global &operator()(int const index0, int const index1) const
		{
			return data[getIndex(index<Rank> {{ index0, index1 }})];
		}

		void synchronize()
		{
		}
	};

	template<int Rank, typename Kernel> inline void parallel_for_each(extent<Rank> const &extent, Kernel const &kernel, int rank, index<Rank> &index)
	{
		for(int i = 0; i < extent[rank]; ++i)
		{
			index[rank] = i;

			if(rank + 1 < Rank)
			{
				parallel_for_each(extent, kernel, rank + 1, index);
			}
			else
			{
				kernel(index);
			}
		}
	}

#if __CXXAMP__
	template <int Rank, typename Kernel> [[amp::kernel]] void kernel_proc(Kernel *data, index<Rank> *index) restrict(amp)
	{
		(*data)(*index);
	}

	extern "C" void amp_execute_kernel(void(*)());
	
	struct pointer_buffer
	{
		void const* Data;
		cl::Buffer Buffer;
	};
	
	template<typename Kernel> struct kernel_info
	{
		char const *Code;
		char const *Entry;
		std::uint32_t BufferCount;
		void (*GetBuffers)(Kernel const* data, pointer_buffer const** buffers);
	};
	
	template<int Rank, typename Kernel> inline void parallel_for_each(extent<Rank> const &extent, Kernel const &kernel)
	{
		auto const &info = *((kernel_info<Kernel> const *(*)())&kernel_proc<Rank, Kernel>)();
		std::cout << "Code: " << std::endl;
		std::cout << info.Code << std::endl;
		std::cout << "Entry: " << std::endl;
		std::cout << info.Entry << std::endl;
		std::cout << "BufferCount: " << std::endl;
		std::cout << info.BufferCount << std::endl;
		std::cout << "GetBuffer: " << std::endl;
		std::cout << (void*)info.GetBuffers << std::endl;
		
		std::cout << "Kernel: " << (void*)&kernel << std::endl;

		pointer_buffer const * buffers[10];
		info.GetBuffers(&kernel, buffers);
		
		for(unsigned i = 0; i < info.BufferCount; ++i)
		{
			std::cout << "Buffer " << i << ": " << (void*)buffers[i]->Data << std::endl;
		}
	}

	extern "C" void * foo = 0;
#else
	template<int Rank, typename Kernel> inline void parallel_for_each(extent<Rank> const &extent, Kernel const &kernel)
	{
		index<Rank> index;
		parallel_for_each(extent, kernel, 0, index);
	}
#endif
}
