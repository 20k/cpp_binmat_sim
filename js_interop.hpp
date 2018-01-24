#ifndef JS_INTEROP_HPP_INCLUDED
#define JS_INTEROP_HPP_INCLUDED

#include "duktape.h"

static duk_ret_t native_print(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
	printf("%s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t native_sleep(duk_context* ctx)
{
    Sleep(1);

    return 0;
}

std::string read_file(const std::string& file)
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

void native_register(duk_context *ctx) {
	duk_push_c_function(ctx, native_print, DUK_VARARGS);
	duk_put_global_string(ctx, "print");

	duk_push_c_function(ctx, native_sleep, 0);
	duk_put_global_string(ctx, "sleep");
}

void debug_stack(duk_context* ctx)
{
    duk_push_context_dump(ctx);

    std::cout << duk_get_string(ctx, -1) << std::endl;

    duk_pop_n(ctx, 1);
}

struct stack_duk;

struct arg_idx
{
    stack_duk& isd;
    int val = 0;
    bool has_state = false;
    bool should_dec = false;
    int n = 0;

    explicit arg_idx(stack_duk& sd);

    arg_idx(const arg_idx& other) = delete;
    arg_idx& operator=(arg_idx& other) = delete;

    arg_idx(arg_idx&& other) noexcept;
    arg_idx& operator=(arg_idx&& other) noexcept;

    void invalidate()
    {
        has_state = false;
    }

    void consume(arg_idx&& other)
    {
        other.has_state = false;
        other.should_dec = false;
        n += other.n;
        other.n = 0;
        has_state = true;
        should_dec = true;
    }

    ~arg_idx();
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
        return function_implicit_call_point - stack_val;
    }

    void load()
    {
        stack_val = saved.back();

        saved.pop_back();

        //return arg_idx(stack_val);
    }

    arg_idx inc()
    {
        return arg_idx(*this);
    }

    void pop_n_internal(int n)
    {
        duk_pop_n(ctx, n);
        stack_val -= n;
    }

    arg_idx push_global_object()
    {
        duk_push_global_object(ctx);

        return arg_idx(*this);
    }

    arg_idx push_string(const std::string& s)
    {
        duk_push_string(ctx, s.c_str());

        return arg_idx(*this);
    }

    arg_idx push_int(int v)
    {
        duk_push_int(ctx, v);

        return arg_idx(*this);
    }

    arg_idx push_boolean(bool v)
    {
        duk_push_boolean(ctx, v);

        return arg_idx(*this);
    }

    bool has_prop_string(arg_idx& offset, const std::string& name)
    {
        int foffset = offset.val - stack_val;

        return duk_has_prop_string(ctx, foffset, name.c_str());
    }

    arg_idx get_prop_string(int offset, const std::string& name)
    {
        duk_get_prop_string(ctx, offset, name.c_str());

        return arg_idx(*this);
    }

    arg_idx get_prop_string(arg_idx& offset, const std::string& name)
    {
        int foffset = offset.val - stack_val;

        return get_prop_string(foffset, name);
    }

    arg_idx get_prop_index(arg_idx& offset, int index)
    {
        int foffset = offset.val - stack_val;

        duk_get_prop_index(ctx, foffset, index);

        return arg_idx(*this);
    }

    int get_length(arg_idx& offset)
    {
        int foffset = offset.val - stack_val;

        return duk_get_length(ctx, foffset);
    }

    void call(int args, int to_remove)
    {
        //make_invalid(std::forward<T>(alist)...);

        int ret = duk_pcall(ctx, args);

        if(ret != DUK_EXEC_SUCCESS)
        {
            printf("error: %s\n", duk_safe_to_string(ctx, -1));
        }

        //stack_val = stack_val - to_remove + 1;

        ///problem is, this is completely wrong
        //arg_idx v = arg_idx(*this);

        //stack_val --;

        //stack_val -= args;

       // return v;
    }

    int get_int(arg_idx& offset)
    {
        int foffset = offset.val - stack_val;

        return duk_get_int(ctx, foffset);
    }

    bool get_boolean(arg_idx& offset)
    {
        int foffset = offset.val - stack_val;

        return duk_get_boolean(ctx, foffset);
    }

    std::string get_string(arg_idx& offset)
    {
        int foffset = offset.val - stack_val;

        return duk_get_string(ctx, foffset);
    }

    arg_idx dup_absolute(int absolute_value)
    {
        int diff = absolute_value - stack_val;

        duk_dup(ctx, diff);

        return arg_idx(*this);
    }
};

arg_idx::arg_idx(stack_duk& sd) : isd(sd)
{
    val = sd.stack_val;
    sd.stack_val++;
    has_state = true;
    should_dec = true;
    n = 1;
}

arg_idx::~arg_idx()
{
    if(has_state)
    {
        duk_pop_n(isd.ctx, n);
        printf("pop\n");
    }

    if(should_dec)
        isd.stack_val -= n;
}

arg_idx::arg_idx(arg_idx&& other) noexcept : isd(other.isd)
{
    other.has_state = false;
    other.should_dec = false;
    n = other.n;
    other.n = 0;
    has_state = true;
    should_dec = true;

    val = other.val;
}

arg_idx& arg_idx::operator=(arg_idx&& other) noexcept
{
    other.has_state = false;
    other.should_dec = false;
    n = other.n;
    other.n = 0;
    has_state = true;
    should_dec = true;

    val = other.val;
}

/*arg_idx& arg_idx::operator=(arg_idx& other) noexcept
{
    isd = other.isd;
    other.has_state = false;

    has_state = true;
    val = other.val;
}*/

arg_idx call_global_function(stack_duk& sd, const std::string& name)
{
    //sd.save();

    arg_idx glob_id = sd.push_global_object();

    ///warning
    ///this can't expire before call happens
    ///maybe eh, we should tie them together
    arg_idx a1 = sd.get_prop_string(glob_id, name);
    //a1.invalidate();
    //a1.should_dec = false;

    sd.call(0, 0);
    a1.consume(std::move(glob_id));


    return a1;

    ///call and get prop str both inc, but call removes one item off stack
    //sd.pop_n(1);

    //sd.stack_val--;

    //sd.load();

    //sd.inc();

    //return ret;
}

/*arg_idx call_implicit_function(stack_duk& sd, const std::string& name)
{
    //duk_get_prop_string(ctx, -1, name.c_str());
    //duk_call(ctx, 0);
    //sd.save();

    arg_idx arg = sd.get_prop_string(-1, name);


    arg_idx ret = sd.call(0);
    //sd.load();

    //sd.inc();

    return ret;
}*/

arg_idx push_arg(stack_duk& sd, const std::string& s)
{
    return sd.push_string(s);
}

arg_idx push_arg(stack_duk& sd, int s)
{
    return sd.push_int(s);
}

arg_idx push_arg(stack_duk& sd, bool s)
{
    return sd.push_boolean(s);
}

arg_idx push_arg(stack_duk& sd, arg_idx& s)
{
    return sd.dup_absolute(s.val);
}

template<typename T, typename... U>
void set_arg(stack_duk& sd, T&& t, U&&... u)
{
    arg_idx arg = push_arg(sd, std::forward<T>(t));
    arg.invalidate();
    arg.should_dec = false;

    set_arg(sd, std::forward<U>(u)...);
}

void set_arg(stack_duk& sd)
{

}

template<typename... T>
void set_args(stack_duk& sd, T&&... t)
{
    set_arg(sd, std::forward<T>(t)...);
}

template<typename... T>
arg_idx call_function_from_absolute(stack_duk& sd, const std::string& name, T&&... arg_offsets)
{
    //sd.save();
    arg_idx arg = sd.get_prop_string(sd.get_function_offset(), name);

    set_args(sd, arg_offsets...);

    int len = sizeof...(arg_offsets);

    //arg.invalidate();
    //arg.should_dec = false;

    sd.call(len, 0);
    sd.stack_val -= len;

    return arg;

    //sd.stack_val -= len;

    //ret.consume(std::move(arg));

    //sd.load();

    //sd.inc();

    //return ret;
}

template<typename... T>
arg_idx call_function_from(stack_duk& sd, const std::string& name, arg_idx& from, T&&... arg_offsets)
{
    //sd.save();

    arg_idx callsite = sd.get_prop_string(from, name);

    //callsite.invalidate();
    //callsite.should_dec = false;

    printf("prestack %i\n", sd.stack_val);

    set_args(sd, std::forward<T>(arg_offsets)...);

    printf("poststack %i\n", sd.stack_val);

    int len = sizeof...(arg_offsets);


    sd.call(len, 0);
    sd.stack_val -= len;
    //sd.stack_val -= len;

    //ret.consume(std::move(callsite));

    return callsite;


    //sd.load();
    //sd.inc();

    //return arg_idx(sd.stack_val - 1);
}

void register_function(stack_duk& sd, const std::string& function_str, const std::string& function_name)
{
    std::string test_js = "var global = new Function(\'return this;\')();\n"
                          "global." + function_name + " = " + function_str + ";\n";//\n print(global.test)"

    //std::cout << "TEST " << test_js << "\n\n\n" << std::endl;

    int pc = duk_peval_string(sd.ctx, test_js.c_str());
    //sd.inc();

    //sd.stack_val++;

    if(pc)
    {
        printf("eval failed: %s\n", duk_safe_to_string(sd.ctx, -1));
        exit(0);
    }
}

duk_context* js_interop_startup()
{
    duk_context *ctx = duk_create_heap_default();

    native_register(ctx);

    return ctx;
}

#endif // JS_INTEROP_HPP_INCLUDED
