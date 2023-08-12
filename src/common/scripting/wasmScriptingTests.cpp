#include <filesystem>
#include <fstream>
#include <iostream>
#include "module_metadata_parser.h"
#include "wasmer.h"

int main()
{
    auto metadata = module_metadata_parser::build_module_metadata(std::filesystem::current_path().string().c_str());

    std::cout << "Found components: \n";
    for(int i = 0; i < metadata->components.length; ++i)
        std::cout << "\t" << i << ":" << metadata->components.data[i].name.data << '\n';
    std::cout << "Found systems: \n";
    for(int i = 0; i < metadata->systems.length; ++i)
        std::cout << "\t" << i << ":" << metadata->systems.data[i].name.data << std::endl;

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
    wasm_config_t* config = wasm_config_new();

    wasmer_features_t* features = wasmer_features_new();
    wasmer_features_module_linking(features, true);
    wasm_config_set_features(config, features);

    wasm_engine_t* engine = wasm_engine_new_with_config(config);

    wasm_store_t* store = wasm_store_new(engine);

    printf("Compiling module\n");
    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);

    if(!module)
    {
        std::cerr << "Failed to compile module" << std::endl;
        return 1;
    }

    wasm_byte_vec_delete(&wasm_bytes);

    printf("Reading imports\n");
    wasm_importtype_vec_t importTypes;
    wasm_module_imports(module, &importTypes);
    printf("Found %zu imports:\n", importTypes.size);
    for(int i = 0; i < importTypes.size; ++i)
    {
        wasm_importtype_t* def = importTypes.data[i];
        const wasm_externtype_t* e = wasm_importtype_type(def);

        const wasm_name_t* name = wasm_importtype_name(def);
        printf("  Import %d: %s\n", i, name->data);
        switch((wasm_externkind_enum)wasm_externtype_kind(e))
        {
            case WASM_EXTERN_FUNC:
            {
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
                        default:
                            printf("      %d: unknown\n", p);
                            break;
                    }
                }
                printf("    Results:\n");
                auto* results = wasm_functype_results(f);
                for(int p = 0; p < results->size; ++p)
                {
                    auto* result = results->data[p];
                    switch((wasm_valkind_enum)wasm_valtype_kind(result))
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
                        default:
                            printf("      %d: unknown\n", p);
                            break;
                    }
                }
                break;
            }
            case WASM_EXTERN_GLOBAL:
            {
                auto* g = wasm_externtype_as_globaltype_const(e);
                printf("    Type: ");
                switch((wasm_valkind_enum)wasm_valtype_kind(wasm_globaltype_content(g)))
                {
                    case WASM_I32:
                        printf("i32\n");
                        break;
                    case WASM_I64:
                        printf("i64\n");
                    break;
                    case WASM_F32:
                        printf("f32\n");
                        break;
                    case WASM_F64:
                        printf("f64\n");
                        break;
                    default:
                        printf("unknown\n");
                        break;
                }
            }
            case WASM_EXTERN_MEMORY:
            {
                auto* m = wasm_externtype_as_memorytype_const(e);
                printf("    Type: %d pages\n", wasm_memorytype_limits(m)->max);
                break;
            }
        }
    }


    printf("Reading exports\n");
    wasm_exporttype_vec_t exportTypes;
    wasm_module_exports(module, &exportTypes);

    if(exportTypes.size == 0)
    {
        std::cerr << "No exports found! " << exportTypes.size << std::endl;
        return 1;
    }

    printf("Found %zu exports:\n", exportTypes.size);
    int testFuncIndex = -1;
    int testCallExternIndex = -1;
    for(int i = 0; i < exportTypes.size; ++i)
    {
        wasm_exporttype_t* def = exportTypes.data[i];
        const wasm_externtype_t* e = wasm_exporttype_type(def);

        const wasm_name_t* name = wasm_exporttype_name(def);
        if(strncmp(name->data, "test_function", strlen("test_function")) == 0)
            testFuncIndex = i;
        if(strncmp(name->data, "test_extern_call", strlen("test_extern_call")) == 0)
            testCallExternIndex = i;

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



    wasm_valtype_vec_t test_external_function_args;
    wasm_valtype_vec_t test_external_function_ret;
    wasm_valtype_vec_new_empty(&test_external_function_ret);
    wasm_valtype_vec_new_uninitialized(&test_external_function_ret, 1);
    test_external_function_ret.data[0] = wasm_valtype_new(WASM_I32);
    auto* test_external_function_type = wasm_functype_new(&test_external_function_args, &test_external_function_ret);
    wasm_func_t* test_external_function = wasm_func_new(store, test_external_function_type,
                                                        [](const wasm_val_vec_t* args, wasm_val_vec_t* results) -> wasm_trap_t*{
        results->data[0].of.i32 = 69;
        results->data[0].kind = WASM_I32;
        return nullptr;
    });

    wasm_extern_t* imports[1] = { wasm_func_as_extern(test_external_function) };
    wasm_extern_vec_t import_object = WASM_ARRAY_VEC(imports);

    printf("Instantiating module\n");
    wasm_trap_t* instance_error = NULL;
    wasm_instance_t* instance = wasm_instance_new(store, module, &import_object, &instance_error);

    if(!instance)
    {
        wasm_message_t err;
        err.data = new wasm_byte_t[wasmer_last_error_length() + 1];
        err.size = wasmer_last_error_length();
        wasmer_last_error_message(err.data, err.size);
        std::cerr << "Failed to instantiate module: " << err.data << std::endl;
        return 1;
    }

    if(testFuncIndex == -1)
    {
        std::cerr << "Failed to find test_function() in module" << std::endl;
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

    wasm_func_t* test_function = wasm_extern_as_func(externs.data[testFuncIndex]);
    wasm_val_t args_val[1] = { WASM_I32_VAL(42) };
    wasm_val_t results_val[1] = {WASM_INIT_VAL};
    wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
    wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);
    if(auto t = wasm_func_call(test_function, &args, &results))
    {
        wasm_message_t msg;
        wasm_trap_message(t, &msg);
        printf("Error calling function: %s\n", msg.data);
    }

    printf("test_function() returned %d\n", results_val[0].of.i32);


    wasm_func_t* test_extern_call_function = wasm_extern_as_func(externs.data[testCallExternIndex]);
    wasm_val_t empty_args_val[0] = { };
    wasm_val_vec_t empty_args = WASM_ARRAY_VEC(empty_args_val);
    if(auto t = wasm_func_call(test_extern_call_function, &empty_args, &results))
    {
        wasm_message_t msg;
        wasm_trap_message(t, &msg);
        printf("Error calling function: %s\n", msg.data);
    }

    printf("test_extern_call() returned %d\n", results_val[0].of.i32);

    wasm_extern_vec_delete(&externs);
    wasm_instance_delete(instance);
    wasm_module_delete(module);
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    return 0;
}
