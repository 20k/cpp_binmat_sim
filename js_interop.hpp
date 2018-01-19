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

static duk_ret_t native_adder(duk_context *ctx) {
	int i;
	int n = duk_get_top(ctx);  /* #args */
	double res = 0.0;

	for (i = 0; i < n; i++) {
		res += duk_to_number(ctx, i);
	}

	duk_push_number(ctx, res);
	return 1;  /* one return value */
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

void print_register(duk_context *ctx) {
	duk_push_c_function(ctx, native_print, DUK_VARARGS);
	duk_put_global_string(ctx, "print");
}

void debug_stack(duk_context* ctx)
{
    duk_push_context_dump(ctx);

    std::cout << duk_get_string(ctx, -1) << std::endl;

    duk_pop_n(ctx, 1);
}

struct arg_idx
{
    int val = 0;

    arg_idx(int v)
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

        return stack_val;
    }

    arg_idx inc()
    {
        stack_val++;

        return stack_val;
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

        return stack_val-1;
    }

    arg_idx push_string(const std::string& s)
    {
        duk_push_string(ctx, s.c_str());

        inc();

        return stack_val-1;
    }

    arg_idx push_int(int v)
    {
        duk_push_int(ctx, v);

        inc();

        return stack_val-1;
    }

    arg_idx get_prop_string(int offset, const std::string& name)
    {
        duk_get_prop_string(ctx, offset, name.c_str());

        inc();

        return stack_val-1;
    }

    void call(int args)
    {
        duk_call(ctx, args);
    }

    arg_idx dup_absolute(int absolute_value)
    {
        int diff = absolute_value - stack_val - 1;

        duk_dup(ctx, diff);

        inc();

        return stack_val-1;
    }
};

arg_idx call_global_function(stack_duk& sd, const std::string& name)
{
    sd.save();

    sd.push_global_object();

    sd.get_prop_string(-1, name);

    sd.call(0);

    sd.load();

    return sd.inc();
}

arg_idx call_implicit_function(stack_duk& sd, const std::string& name)
{
    //duk_get_prop_string(ctx, -1, name.c_str());
    //duk_call(ctx, 0);
    sd.save();

    sd.get_prop_string(-1, name);
    sd.call(0);
    sd.load();

    return sd.inc();
}

void push_arg(stack_duk& sd, const std::string& s)
{
    sd.push_string(s);
}

void push_arg(stack_duk& sd, int s)
{
    sd.push_int(s);
}

void push_arg(stack_duk& sd, arg_idx s)
{
    sd.dup_absolute(s.val);
}

template<typename T, typename... U>
void set_arg(stack_duk& sd, T t, U... u)
{
    push_arg(sd, std::forward<T>(t));
    set_arg(sd, std::forward<U>(u)...);
}

void set_arg(stack_duk& sd)
{

}

template<typename... T>
void set_args(stack_duk& sd, T... t)
{
    set_arg(sd, std::forward<T>(t)...);
}

template<typename... T>
arg_idx call_function_from_absolute(stack_duk& sd, const std::string& name, T... arg_offsets)
{
    sd.save();
    sd.get_prop_string(sd.get_function_offset(), name);

    //duk_get_prop_string(ctx, offset, name.c_str());

    set_args(sd, arg_offsets...);

    int len = sizeof...(arg_offsets);

    int ret = duk_pcall(sd.ctx, len);

    if(ret != DUK_EXEC_SUCCESS)
    {
        printf("error: %s\n", duk_safe_to_string(sd.ctx, -1));

        //duk_pop(ctx);

        sd.pop_n(1);
    }

    sd.load();

    return sd.inc();
}

void register_function(stack_duk& sd, const std::string& function_str, const std::string& function_name)
{
    std::string test_js = "var global = new Function(\'return this;\')();\n"
                           + function_str + "\n"
                           "global." + function_name + " = " + function_name + ";\n";//\n print(global.test)"

    int pc = duk_peval_string(sd.ctx, test_js.c_str());

    if(pc)
    {
        printf("eval failed: %s\n", duk_safe_to_string(sd.ctx, -1));
    }
}

duk_context* js_interop_startup()
{
    duk_context *ctx = duk_create_heap_default();

    print_register(ctx);

    return ctx;
}

#endif // JS_INTEROP_HPP_INCLUDED
