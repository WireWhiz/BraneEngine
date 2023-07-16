#include <fstream>
#include <iostream>
#include "wasmer.h"

int main()
{
    std::ifstream wasmFile("wasm_crate.wasm", std::ios::binary | std::ios::ate);
    if(!wasmFile.is_open())
    {
        std::cerr << "Failed to open wasm file" << std::endl;
        return 1;
    }

    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new_uninitialized(&wasm_bytes, (int)wasmFile.tellg());
    wasmFile.seekg(0, std::ios::beg);
    wasmFile.read((char*)wasm_bytes.data, wasm_bytes.size);
    wasmFile.close();

    printf("WASM file read\n");

    printf("Allocating memory\n");
    wasm_engine_t* engine = wasm_engine_new();
    wasm_store_t* store = wasm_store_new(engine);

    printf("Compiling module\n");
    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);

    if(!module)
    {
        std::cerr << "Failed to compile module" << std::endl;
        return 1;
    }

    wasm_byte_vec_delete(&wasm_bytes);

    printf("Reading exports\n");
    wasm_exporttype_vec_t exportTypes;
    wasm_module_exports(module, &exportTypes);

    if(exportTypes.size == 0)
    {
        std::cerr << "No exports found! " << exportTypes.size << std::endl;
        return 1;
    }

    printf("Found %zu exports:\n", exportTypes.size);
    for(int i = 0; i < exportTypes.size; ++i)
    {
        wasm_exporttype_t* def = exportTypes.data[i];
        const wasm_externtype_t* e = wasm_exporttype_type(def);

        const wasm_name_t* name = wasm_exporttype_name(def);

        switch((wasm_externkind_enum)wasm_externtype_kind(e))
        {
            case WASM_EXTERN_FUNC:
            {
                printf("  Export %d: (Function) %s \n", i, name->data);
                auto* f = wasm_externtype_as_functype_const(e);
                printf("    Params:\n");
                auto* params = wasm_functype_params(f);
                for(int p = 0; p < params->size; ++p)
                {
                    auto* param = params->data[p];
                    switch((wasm_valkind_enum)wasm_valtype_kind(param))
                    {
                        case WASM_I32:
                            printf("      %d: i32\n", p);
                            break;
                        case WASM_I64:
                            printf("      %d: i64\n", p);
                            break;
                        case WASM_F32:
                            printf("      %d: f32\n", p);
                            break;
                        case WASM_F64:
                            printf("      %d: f64\n", p);
                            break;
                        case WASM_ANYREF:
                            printf("      %d: anyref\n", p);
                            break;
                        case WASM_FUNCREF:
                            printf("      %d: funcref\n", p);
                            break;
                    }
                }
                break;
            }
            case WASM_EXTERN_GLOBAL:
                printf("  Export %d: (Global) %s \n", i, name->data);

                break;
            case WASM_EXTERN_TABLE:
                printf("  Export %d: (Table) %s \n", i, name->data);
                break;
            case WASM_EXTERN_MEMORY:
            {
                printf("  Export %d: (Memory) %s \n", i, name->data);
                break;
            }
        }
    }
    wasm_exporttype_vec_delete(&exportTypes);


    wasm_extern_vec_t import_object = WASM_EMPTY_VEC;

    printf("Instantiating module\n");
    wasm_instance_t* instance = wasm_instance_new(store, module, &import_object, NULL);

    if(!instance)
    {
        std::cerr << "Failed to instantiate module" << std::endl;
        return 1;
    }

    printf("Attempting to call test_function()\n");

    wasm_extern_vec_t externs;
    wasm_instance_exports(instance, &externs);

    if(externs.size == 0)
    {
        std::cerr << "No exports found! " << externs.size << std::endl;
        return 1;
    }
    printf("Found %zu externs:\n", externs.size);

    wasm_func_t* test_function = wasm_extern_as_func(externs.data[1]);
    wasm_val_t args_val[0] = {};
    wasm_val_t results_val[1] = {WASM_INIT_VAL};
    wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
    wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);
    if(auto t =wasm_func_call(test_function, &args, &results))
    {
        wasm_message_t msg;
        wasm_trap_message(t, &msg);
        printf("Error calling function: %s\n", msg.data);
    }

    printf("test_function() returned %d\n", results_val[0].of.i32);

    wasm_extern_vec_delete(&externs);
    wasm_instance_delete(instance);
    wasm_module_delete(module);
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    return 0;
}
