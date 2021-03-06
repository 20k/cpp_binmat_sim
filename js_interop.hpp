#ifndef JS_INTEROP_HPP_INCLUDED
#define JS_INTEROP_HPP_INCLUDED

#include "duktape.h"
#include <iostream>
#include <sstream>
#include <fstream>

#if 0
static duk_ret_t native_print(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
	printf("%s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t native_sleep(duk_context* ctx)
{
    //Sleep(1);

    return 0;
}
#endif // 0

inline
std::string read_file_b(const std::string& file)
{
    FILE *f = fopen(file.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    std::string buffer;
    buffer.resize(fsize + 1);
    fread(&buffer[0], fsize, 1, f);
    fclose(f);

    return buffer;
}

inline
std::string read_file_r(const std::string& file)
{
    FILE *f = fopen(file.c_str(), "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    std::string buffer;
    buffer.resize(fsize + 1);
    fread(&buffer[0], fsize, 1, f);
    fclose(f);

    return buffer;
}

#if 0
inline
std::string read_file(const std::string& file)
{
    std::ifstream t(file);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    return str;
}
#endif // 0

inline
void native_register(duk_context *ctx) {
	/*duk_push_c_function(ctx, native_print, DUK_VARARGS);
	duk_put_global_string(ctx, "print");

	duk_push_c_function(ctx, native_sleep, 0);
	duk_put_global_string(ctx, "sleep");*/
}

template<typename T, typename U>
inline
void inject_c_function(duk_context *ctx, T& t, const std::string& str, U nargs)
{
    duk_push_c_function(ctx, &t, nargs);
	duk_put_global_string(ctx, str.c_str());
}

inline
void debug_stack(duk_context* ctx)
{
    duk_push_context_dump(ctx);

    std::cout << duk_get_string(ctx, -1) << std::endl;

    duk_pop_n(ctx, 1);
}

struct arg_idx
{
    int val = 0;

    explicit arg_idx(int v)
    {
        val = v;
    }
};

struct stack_duk
{
    int stack_val = 0;
    int function_implicit_call_point = 0;
    duk_context* ctx = nullptr;

    std::vector<int> saved;

    void save()
    {
        saved.push_back(stack_val);
    }

    void save_function_call_point()
    {
        function_implicit_call_point = stack_val;
    }

    int get_function_offset()
    {
        return function_implicit_call_point - stack_val - 1;
    }

    arg_idx load()
    {
        stack_val = saved.back();

        saved.pop_back();

        return arg_idx(stack_val);
    }

    arg_idx inc()
    {
        stack_val++;

        return arg_idx(stack_val);
    }

    void pop_n(int n)
    {
        duk_pop_n(ctx, n);
        stack_val -= n;
    }

    arg_idx push_global_object()
    {
        duk_push_global_object(ctx);

        inc();

        return arg_idx(stack_val-1);
    }

    arg_idx push_string(const std::string& s)
    {
        duk_push_string(ctx, s.c_str());

        inc();

        return arg_idx(stack_val-1);
    }

    arg_idx push_int(int v)
    {
        duk_push_int(ctx, v);

        inc();

        return arg_idx(stack_val-1);
    }

    arg_idx push_boolean(bool v)
    {
        duk_push_boolean(ctx, v);

        inc();

        return arg_idx(stack_val-1);
    }

    bool has_prop_string(arg_idx offset, const std::string& name)
    {
        int foffset = offset.val - stack_val;

        return duk_has_prop_string(ctx, foffset, name.c_str());
    }

    arg_idx get_prop_string(int offset, const std::string& name)
    {
        duk_get_prop_string(ctx, offset, name.c_str());

        inc();

        return arg_idx(stack_val-1);
    }

    arg_idx get_prop_string(arg_idx offset, const std::string& name)
    {
        int foffset = offset.val - stack_val;

        return get_prop_string(foffset, name);
    }

    arg_idx get_prop_index(arg_idx offset, int index)
    {
        int foffset = offset.val - stack_val;

        duk_get_prop_index(ctx, foffset, index);

        inc();

        return arg_idx(stack_val-1);
    }

    int get_length(arg_idx offset)
    {
        int foffset = offset.val - stack_val;

        return duk_get_length(ctx, foffset);
    }

    void call(int args)
    {
        int ret = duk_pcall(ctx, args);

        if(ret != DUK_EXEC_SUCCESS)
        {
            printf("error in sdcall: %s\n", duk_safe_to_string(ctx, -1));
        }
    }

    int get_int(arg_idx offset)
    {
        int foffset = offset.val - stack_val;

        return duk_get_int(ctx, foffset);
    }

    bool get_boolean(arg_idx offset)
    {
        int foffset = offset.val - stack_val;

        return duk_get_boolean(ctx, foffset);
    }

    std::string get_string(arg_idx offset)
    {
        int foffset = offset.val - stack_val;

        return duk_get_string(ctx, foffset);
    }

    arg_idx dup_absolute(int absolute_value)
    {
        int diff = absolute_value - stack_val;

        duk_dup(ctx, diff);

        inc();

        return arg_idx(stack_val-1);
    }
};

inline
arg_idx call_global_function(stack_duk& sd, const std::string& name)
{
    //sd.save();

    sd.push_global_object();

    sd.get_prop_string(-1, name);

    sd.call(0);

    ///call and get prop str both inc, but call removes one item off stack
    //sd.pop_n(1);

    //sd.stack_val--;

    //sd.load();

    //sd.inc();

    return arg_idx(sd.stack_val-1);
}

inline
arg_idx call_implicit_function(stack_duk& sd, const std::string& name)
{
    //duk_get_prop_string(ctx, -1, name.c_str());
    //duk_call(ctx, 0);
    sd.save();

    sd.get_prop_string(-1, name);
    sd.call(0);
    sd.load();

    sd.inc();

    return arg_idx(sd.stack_val-1);
}

inline
void push_arg(stack_duk& sd, const std::string& s)
{
    sd.push_string(s);
}

inline
void push_arg(stack_duk& sd, int s)
{
    sd.push_int(s);
}

inline
void push_arg(stack_duk& sd, bool s)
{
    sd.push_boolean(s);
}

inline
void push_arg(stack_duk& sd, arg_idx s)
{
    sd.dup_absolute(s.val);
}

template<typename T, typename... U>
inline
void set_arg(stack_duk& sd, T t, U... u)
{
    push_arg(sd, std::forward<T>(t));
    set_arg(sd, std::forward<U>(u)...);
}

inline
void set_arg(stack_duk& sd)
{

}

template<typename... T>
inline
void set_args(stack_duk& sd, T... t)
{
    set_arg(sd, std::forward<T>(t)...);
}

template<typename... T>
inline
arg_idx call_function_from_absolute(stack_duk& sd, const std::string& name, T... arg_offsets)
{
    sd.save();
    sd.get_prop_string(sd.get_function_offset(), name);

    set_args(sd, arg_offsets...);

    int len = sizeof...(arg_offsets);

    int ret = duk_pcall(sd.ctx, len);

    if(ret != DUK_EXEC_SUCCESS)
    {
        printf("error: %s\n", duk_safe_to_string(sd.ctx, -1));
    }

    sd.load();

    sd.inc();

    return arg_idx(sd.stack_val - 1);
}

template<typename... T>
inline
arg_idx call_function_from(stack_duk& sd, const std::string& name, arg_idx from, T... arg_offsets)
{
    sd.save();

    sd.get_prop_string(from, name);

    set_args(sd, arg_offsets...);

    int len = sizeof...(arg_offsets);

    int ret = duk_pcall(sd.ctx, len);

    if(ret != DUK_EXEC_SUCCESS)
    {
        printf("error: %s\n", duk_safe_to_string(sd.ctx, -1));
    }


    sd.load();
    sd.inc();

    return arg_idx(sd.stack_val - 1);
}

inline
void register_function(stack_duk& sd, const std::string& function_str, const std::string& function_name)
{
    std::string test_js = "var global = new Function(\'return this;\')();\n"
                          "global." + function_name + " = " + function_str + ";\n";//\n print(global.test)"

    //std::cout << "TEST " << test_js << "\n\n\n" << std::endl;

    int pc = duk_peval_string(sd.ctx, test_js.c_str());
    sd.inc();

    if(pc)
    {
        printf("eval failed: %s\n", duk_safe_to_string(sd.ctx, -1));
        exit(0);
    }
}

inline
duk_context* js_interop_startup()
{
    duk_context *ctx = duk_create_heap_default();

    native_register(ctx);

    return ctx;
}

inline
void js_interop_shutdown(duk_context* ctx)
{
    duk_destroy_heap(ctx);
}

#endif // JS_INTEROP_HPP_INCLUDED
