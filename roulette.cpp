#include <utility>
#include <vector>
#include <CL/cl.hpp>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <memory>
#include <sstream>

extern "C" {
#include "miner.h"
};

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

extern int opt_device;
extern int opt_worksize;
extern int opt_intensity;

inline void checkErr(cl_int err, const char * name)
{
	if (err != CL_SUCCESS) {
		std::cerr << "ERROR: " << name
			<< " (" << err << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

class RoulettecoinMiner
{
private:
	RoulettecoinMiner();
public:
	static RoulettecoinMiner& getInstance()
        {
		static RoulettecoinMiner m;
		return m;
        }
        std::auto_ptr<cl::Program> program;
        std::auto_ptr<cl::Context> context;
        std::vector<cl::Device> devices;
};

RoulettecoinMiner::RoulettecoinMiner()
{
	cl_int err;
	std::vector< cl::Platform > platformList;
	cl::Platform::get(&platformList);
	checkErr(platformList.size()!=0 ? CL_SUCCESS : -1, "cl::Platform::get");
	std::cerr << "Platform number is: " << platformList.size() << std::endl;std::string platformVendor;
	platformList[0].getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &platformVendor);
	std::cerr << "Platform is by: " << platformVendor << "\n";
	cl_context_properties cprops[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[0])(), 0};
	context.reset(new cl::Context( CL_DEVICE_TYPE_GPU, cprops, NULL, NULL, &err));
	checkErr(err, "Conext::Context()"); 

	devices = context->getInfo<CL_CONTEXT_DEVICES>();
	checkErr(devices.size() > 0 ? CL_SUCCESS : -1, "devices.size() > 0");

	std::ifstream file("kernel/roulettecoin.cl");
	checkErr(file.is_open() ? CL_SUCCESS:-1, "roulettecoin.cl");
	std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));
	program.reset(new cl::Program(*context, source));
        std::stringstream buildArgs;
	buildArgs << "-Ikernel -DWORKSIZE=" << opt_worksize << " -DNUMHASH=" << (1 << (opt_intensity - 4));
	std::string buildArgsStr = buildArgs.str();
	err = program->build(devices, buildArgsStr.c_str());
	if(err != CL_SUCCESS)
	{
		std::cout << "Build Log:\t " << program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
		checkErr(err, "Program::build()");
	}
}

extern "C" {

int scanhash_roulette(int thr_id, uint32_t *pdata, const uint32_t *ptarget,
	uint32_t max_nonce, unsigned long *hashes_done, int thread_id)
{
	uint32_t nHashes = 1 << (opt_intensity - 4);
        unsigned int loops = 16 * nHashes;
        unsigned int gsize = 64;

	const uint32_t first_nonce = pdata[19];
	const uint32_t Htarg = ptarget[7];

	uint32_t hash64[8] __attribute__((aligned(32)));
	uint32_t endiandata[32];

	//we need bigendian data...
	int kk=0;
	for (; kk < 32; kk++)
	{
		be32enc(&endiandata[kk], ((uint32_t*)pdata)[kk]);
	};

	cl_int err;
	RoulettecoinMiner& m = RoulettecoinMiner::getInstance();

	cl::Buffer inCL(*m.context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 88, endiandata, &err);
	checkErr(err, "Buffer::Buffer()");

	unsigned int countersH[16 * 16] = { 0 };

	cl::Buffer inputCL(*m.context, CL_MEM_READ_WRITE, (16 + 2) * nHashes * (64 + 8), NULL, &err);
	checkErr(err, "Buffer::Buffer()");
	cl::Buffer outputCL(*m.context, CL_MEM_READ_WRITE, (16 + 2) * nHashes * (64 + 8), NULL, &err);
	checkErr(err, "Buffer::Buffer()");
	cl::Buffer countersCL(*m.context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, (16 + 1) * sizeof(*countersH), countersH, &err);
	checkErr(err, "Buffer::Buffer()");

        unsigned int outH[256] = { 0 };
        cl::Buffer outCL(*m.context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, 256 * 4, outH, &err);
        checkErr(err, "Buffer::Buffer()");

	cl::Kernel kernel(*m.program, "search", &err);
	checkErr(err, "Kernel::Kernel()");
	err = kernel.setArg(0, inCL);
	checkErr(err, "Kernel::setArg()");
	err = kernel.setArg(1, inputCL);
	checkErr(err, "Kernel::setArg()");
	err = kernel.setArg(2, countersCL);
	checkErr(err, "Kernel::setArg()");

	cl::Kernel algos[16];
	for(int i = 0; i < 16; i++)
	{
		char kername[] = "hash0";
		char kersuffixes[] = { '0', '1', '2', '3', '4', '5', '6', '7',
				       '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		kername[4] = kersuffixes[i];
		algos[i] = cl::Kernel(*m.program, kername, &err);

		checkErr(err, "Kernel::Kernel()");
		err = algos[i].setArg(0, inputCL);
		checkErr(err, "Kernel::setArg()");
		err = algos[i].setArg(1, outputCL);
		checkErr(err, "Kernel::setArg()");
	}

	cl::Kernel shuffler(*m.program, "shuffler", &err);
	checkErr(err, "Kernel::Kernel()");
	err = shuffler.setArg(0, outputCL);
	checkErr(err, "Kernel::setArg()");
	err = shuffler.setArg(1, inputCL);
	checkErr(err, "Kernel::setArg()");
	err = shuffler.setArg(2, countersCL);
	checkErr(err, "Kernel::setArg()");
	err = shuffler.setArg(3, 0);
	checkErr(err, "Kernel::setArg()");
	err = shuffler.setArg(4, 0);
	checkErr(err, "Kernel::setArg()");

	cl::Kernel getsolutions(*m.program, "getsolutions", &err);
	checkErr(err, "Kernel::Kernel()");
	err = getsolutions.setArg(0, outputCL);
	checkErr(err, "Kernel::setArg()");
	err = getsolutions.setArg(1, outCL);
	checkErr(err, "Kernel::setArg()");
	err = getsolutions.setArg(2, ((uint64_t) ptarget[7] << 32) | ptarget[6]);
	checkErr(err, "Kernel::setArg()");

	cl::Kernel clearer(*m.program, "clearer", &err);
	checkErr(err, "Kernel::Kernel()");
	err = clearer.setArg(0, inputCL);
	checkErr(err, "Kernel::Kernel()");

	cl::CommandQueue queue(*m.context, m.devices[(opt_device == -1 ? thread_id % m.devices.size() : opt_device)], CL_QUEUE_PROFILING_ENABLE, &err);
	checkErr(err, "CommandQueue::CommandQueue()");

	cl::Event event;

	err = queue.enqueueNDRangeKernel(clearer, cl::NullRange, cl::NDRange(loops), cl::NDRange(gsize), NULL, NULL);
	checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");

	err = queue.enqueueNDRangeKernel( kernel, cl::NDRange(first_nonce), cl::NDRange(loops), cl::NDRange(gsize), NULL, NULL);
	checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");

	for(int j = 0; j < 16; j++)
	{
		for(int i = 0; i < 16; i++)
		{
			err = queue.enqueueNDRangeKernel(algos[i], cl::NullRange, cl::NDRange(loops >> 4), cl::NDRange(gsize), NULL, NULL);
			checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");
		}

		err = queue.enqueueNDRangeKernel(clearer, cl::NullRange, cl::NDRange(loops), cl::NDRange(gsize), NULL, NULL);
		checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");

		for(int i = 0; j != 15 && i < 16; i++)
		{
			err = shuffler.setArg(3, j + 1);
			checkErr(err, "Kernel::setArg()");
			err = shuffler.setArg(4, i);
			checkErr(err, "Kernel::setArg()");

			err = queue.enqueueNDRangeKernel(shuffler, cl::NullRange, cl::NDRange(loops >> 4), cl::NDRange(gsize), NULL, NULL);
			checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");
		}
	}

	err = queue.enqueueNDRangeKernel(getsolutions, cl::NullRange, cl::NDRange(loops), cl::NDRange(gsize), NULL, NULL);
	checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");

	err = queue.enqueueReadBuffer(outCL, CL_TRUE, 0, 256 * 4, outH);
	checkErr(err, "ComamndQueue::enqueueReadBuffer()");

	if(outH[255] > 0) {
		pdata[19] = swab32(outH[0]);
		*hashes_done = loops;
		return true;
	}

	*hashes_done = loops;
	pdata[19] += loops;
	return 0;
}

};

