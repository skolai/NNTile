#pragma once

#include <stdexcept>
#include <memory>
#include <iostream>
#include <starpu.h>

namespace nntile
{

class Starpu: public starpu_conf
{
    static struct starpu_conf _init_conf()
    {
        struct starpu_conf conf;
        // This function either returns 0 or aborts the program
        int ret = starpu_conf_init(&conf);
        // LCOV_EXCL_START
        if(ret != 0)
        {
            throw std::runtime_error("Error in starpu_conf_init()");
        }
        return conf;
        // LCOV_EXCL_STOP
    }
public:
    Starpu(const struct starpu_conf &conf):
        starpu_conf(conf)
    {
        if(starpu_is_initialized() != 0)
        {
            throw std::runtime_error("Starpu was already initialized");
        }
        int ret = starpu_init(this);
        if(ret != 0)
        {
            throw std::runtime_error("Error in starpu_init()");
        }
    }
    Starpu():
        Starpu(_init_conf())
    {
    }
    ~Starpu()
    {
        starpu_task_wait_for_all();
        starpu_shutdown();
    }
    Starpu(const Starpu &) = delete;
    Starpu(Starpu &&) = delete;
    Starpu &operator=(const Starpu &) = delete;
    Starpu &operator=(Starpu &&) = delete;
};

//! StarPU data handle as a shared pointer to its internal state
//
// This class takes the ownership of the data handle. That said, it unregisters
// the data handle automatically at the end of lifetime.
class StarpuHandle
{
    //! Shared handle itself
    std::shared_ptr<struct _starpu_data_state> handle;
    //! Deleter function for starpu_data_handle_t
    static void _handle_deleter(starpu_data_handle_t ptr)
    {
        starpu_data_unregister(ptr);
    }
public:
    //! Default constructor with no handle
    StarpuHandle() = default;
    //! Constructor owns registered handle and unregisters it when needed
    StarpuHandle(starpu_data_handle_t handle_):
        handle(handle_, _handle_deleter)
    {
    }
    //! Destructor is virtual as this is a base class
    virtual ~StarpuHandle() = default;
    //! Convert to starpu_data_handle_t
    operator starpu_data_handle_t() const
    {
        return handle.get();
    }
    //! Get pointer to local data if corresponding interface supports it
    const void *get_local_ptr() const
    {
        return starpu_data_get_local_ptr(handle.get());
    }
    //! Invalidate handle
    void invalidate() const
    {
        starpu_data_invalidate(handle.get());
    }
    //! Invalidate handle
    void invalidate_submit() const
    {
        starpu_data_invalidate_submit(handle.get());
    }
    //! Acquire data
    void acquire(enum starpu_data_access_mode mode) const
    {
        starpu_data_acquire(handle.get(), mode);
    }
    //! Release acquired data
    void release() const
    {
        starpu_data_release(handle.get());
    }
    //! Advise to flush from GPU to main memory
    void wont_use() const
    {
        starpu_data_wont_use(handle.get());
    }
};

//! Class for StarPU handle for easy registration and unregistration
class StarpuVariableHandle: public StarpuHandle
{
    //! Register variable for starpu-owned memory
    static starpu_data_handle_t _reg_data(size_t size)
    {
        starpu_data_handle_t tmp;
        starpu_variable_data_register(&tmp, -1, 0, size);
        return tmp;
    }
    //! Register variable
    static starpu_data_handle_t _reg_data(uintptr_t ptr, size_t size)
    {
        starpu_data_handle_t tmp;
        starpu_variable_data_register(&tmp, STARPU_MAIN_RAM, ptr, size);
        return tmp;
    }
public:
    //! Constructor for variable that is (de)allocated by starpu
    StarpuVariableHandle(size_t size):
        StarpuHandle(_reg_data(size))
    {
    }
    //! Constructor for variable that is (de)allocated by user
    StarpuVariableHandle(uintptr_t ptr, size_t size):
        StarpuHandle(_reg_data(ptr, size))
    {
    }
};

} // namespace nntile

