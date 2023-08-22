#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include "module_metadata_parser.h"
#include "wasmer.h"
#include "robin_hood.h"

struct TestCompoennt
{
    bool a;
    int32_t b;
    float c;
};

wasm_memory_t* instance_memory = nullptr;

using ImportMap = robin_hood::unordered_map<std::string, wasm_extern_t*>;

void print_externtype_info(const wasm_externtype_t* e)
{
    switch((wasm_externkind_enum)wasm_externtype_kind(e))
    {
        case WASM_EXTERN_FUNC:
        {
            printf("    Type: (Function)\n");
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
            printf("    Type: (Global)\n");
            auto* g = wasm_externtype_as_globaltype_const(e);
            printf("    Data: ");
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
            break;
        }
        case WASM_EXTERN_MEMORY:
        {
            printf("    Type: (Memory)\n");
            auto* m = wasm_externtype_as_memorytype_const(e);
            auto limits = wasm_memorytype_limits(m);
            printf("    Min size: %d pages\n", limits->min);
            if(limits->max != wasm_limits_max_default)
                printf("    Max size: %d pages\n", limits->max);
            else
                printf("    Max size: unlimited\n");
            break;
        }
    }
}

struct WasmModule
{
    wasm_module_t* module = nullptr;

    struct Import
    {
        std::string name;
        wasm_externkind_enum kind = WASM_EXTERN_FUNC;
        wasm_importtype_t* type = nullptr;
    };
    std::vector<Import> imports;
    wasm_importtype_vec_t importTypes;
    wasm_exporttype_vec_t exportTypes;

    struct Export
    {
        uint32_t index = 0;
        wasm_exporttype_t* type = nullptr;
    };
    std::unordered_map<std::string, Export> exports;

    WasmModule()
    {
        wasm_importtype_vec_new_empty(&importTypes);
        wasm_exporttype_vec_new_empty(&exportTypes);
    }

    WasmModule(const WasmModule& other) = delete;
    WasmModule(WasmModule&& other)
    {
        module = other.module;
        imports = std::move(other.imports);
        importTypes = other.importTypes;
        exportTypes = other.exportTypes;
        exports = std::move(other.exports);

        other.module = nullptr;
    }

    ~WasmModule()
    {
        if(module)
        {
            wasm_module_delete(module);
            wasm_importtype_vec_delete(&importTypes);
            wasm_exporttype_vec_delete(&exportTypes);
        }
    }


    bool isValid() const
    {
        return module != nullptr;
    }

    static WasmModule fromFile(std::string path, wasm_store_t* store)
    {
        std::ifstream wasmFile(path, std::ios::binary | std::ios::ate);
        if(!wasmFile.is_open())
        {
            std::cerr << "Failed to open " << path << std::endl;
            return WasmModule{};
        }

        wasm_byte_vec_t wasm_bytes;
        wasm_byte_vec_new_uninitialized(&wasm_bytes, (int)wasmFile.tellg());
        wasmFile.seekg(0, std::ios::beg);
        wasmFile.read((char*)wasm_bytes.data, wasm_bytes.size);
        wasmFile.close();

        if(!wasm_module_validate(store, &wasm_bytes))
        {
            wasm_message_t err;
            err.data = new wasm_byte_t[wasmer_last_error_length() + 1];
            err.size = wasmer_last_error_length();
            wasmer_last_error_message(err.data, err.size);
            std::cerr << "Failed to validate module:" << err.data << std::endl;
            delete err.data;
            return WasmModule{};
        }

        wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
        wasm_byte_vec_delete(&wasm_bytes);

        if(!module)
        {
            wasm_message_t err;
            err.data = new wasm_byte_t[wasmer_last_error_length() + 1];
            err.size = wasmer_last_error_length();
            wasmer_last_error_message(err.data, err.size);
            std::cerr << "Failed to compile module:" << err.data << std::endl;
            delete err.data;
            return WasmModule{};
        }

        WasmModule output{};
        output.module = module;

        wasm_module_imports(module, &output.importTypes);
        for(int i = 0; i < output.importTypes.size; ++i)
        {
            wasm_importtype_t* imptype = output.importTypes.data[i];
            Import imp{};
            imp.type = imptype;

            const wasm_name_t* name = wasm_importtype_name(imptype);
            imp.name = std::string{name->data, name->size};
            const wasm_externtype_t* e = wasm_importtype_type(imptype);
            imp.kind = (wasm_externkind_enum)wasm_externtype_kind(e);
            output.imports.push_back(imp);
        }

        wasm_module_exports(module, &output.exportTypes);
        for(int i = 0; i < output.exportTypes.size; ++i)
        {
            wasm_exporttype_t* exptype = output.exportTypes.data[i];
            Export exp{};
            exp.type = exptype;
            const wasm_name_t* name = wasm_exporttype_name(exptype);
            exp.index = i;
            output.exports.insert({std::string{name->data, name->size}, exp});
        }

        return output;
    }

    void printMetadata()
    {
        printf("Reading imports\n");
        printf("Found %zu imports:\n", importTypes.size);
        for(int i = 0; i < importTypes.size; ++i)
        {
            wasm_importtype_t* def = importTypes.data[i];
            const wasm_externtype_t* e = wasm_importtype_type(def);

            const wasm_name_t* name = wasm_importtype_name(def);
            printf("  Import %d: %s\n", i, std::string{name->data, name->size}.c_str());
            print_externtype_info(e);
        }

        printf("Reading exports\n");

        printf("Found %zu exports:\n", exportTypes.size);
        for(int i = 0; i < exportTypes.size; ++i)
        {
            wasm_exporttype_t* def = exportTypes.data[i];
            const wasm_externtype_t* e = wasm_exporttype_type(def);

            const wasm_name_t* name = wasm_exporttype_name(def);
            printf("  Export %d: %s\n", i, std::string{name->data, name->size}.c_str());

            print_externtype_info(e);
        }
    }

    wasm_extern_vec_t buildImportList(const ImportMap& map)
    {
        wasm_extern_vec_t output{};
        wasm_extern_vec_new_uninitialized(&output, imports.size());
        for(int i = 0; i < output.size; ++i)
        {
            auto& imp = imports[i];
            auto it = map.find(imp.name);
            if(it == map.end())
            {
                std::cerr << "Failed to find import: " << imp.name << std::endl;
                continue;
            }
            output.data[i] = it->second;
        }

        return output;
    }
};

int main()
{
    auto metadata = module_metadata_parser::build_module_metadata(std::filesystem::current_path().string().c_str());

    std::cout << "Found components: \n";
    for(int i = 0; i < metadata->components.length; ++i)
        std::cout << "\t" << i << ":" << metadata->components.data[i].name.data << '\n';
    std::cout << "Found systems: \n";
    for(int i = 0; i < metadata->systems.length; ++i)
        std::cout << "\t" << i << ":" << metadata->systems.data[i].name.data << std::endl;

    printf("Allocating memory\n");
    wasm_config_t* config = wasm_config_new();

    wasmer_features_t* features = wasmer_features_new();
    //wasmer_features_module_linking(features, true);
    wasmer_features_bulk_memory(features, true);
    wasmer_features_threads(features, true);
    wasm_config_set_features(config, features);

    wasm_engine_t* engine = wasm_engine_new_with_config(config);
    wasm_store_t* store = wasm_store_new(engine);

    wasm_limits_t instance_memory_limits;
    instance_memory_limits.min = 30;
    instance_memory_limits.max = 32769;
    wasm_memorytype_t* instance_memory_type = wasm_memorytype_new(&instance_memory_limits);
    instance_memory = wasm_memory_new(store, instance_memory_type);

    ImportMap importsMap;
    importsMap.insert({"memory", wasm_memory_as_extern(instance_memory)});

    {
        wasm_valtype_vec_t test_external_function_args;
        wasm_valtype_vec_t test_external_function_ret;
        wasm_valtype_vec_new_empty(&test_external_function_args);
        wasm_valtype_vec_new_uninitialized(&test_external_function_ret, 1);
        test_external_function_ret.data[0] = wasm_valtype_new(WASM_I32);
        auto* test_external_function_type =
            wasm_functype_new(&test_external_function_args, &test_external_function_ret);

        wasm_func_t* test_external_function = wasm_func_new(store,
                          test_external_function_type,
                          [](const wasm_val_vec_t* args, wasm_val_vec_t* results) -> wasm_trap_t* {
            results->data[0].of.i32 = 69;
            results->data[0].kind = WASM_I32;
            return nullptr;
        });
        importsMap.insert({"test_external_function", wasm_func_as_extern(test_external_function)});
    }

    {
        wasm_valtype_vec_t test_external_function_args;
        wasm_valtype_vec_t test_external_function_ret;
        wasm_valtype_vec_new_uninitialized(&test_external_function_args, 1);
        test_external_function_args.data[0] = wasm_valtype_new(WASM_I32);
        wasm_valtype_vec_new_empty(&test_external_function_ret);
        auto* test_external_function_type =
            wasm_functype_new(&test_external_function_args, &test_external_function_ret);
        wasm_func_t* be_print = wasm_func_new(store, test_external_function_type,
                                               [](const wasm_val_vec_t* args, wasm_val_vec_t* results) -> wasm_trap_t* {
            assert(args->size == 1);
            assert(args->data[0].kind == WASM_I32);
            auto* mod_data = wasm_memory_data(instance_memory);

            std::cout << &mod_data[args->data[0].of.i32] << std::endl;

            return nullptr;
        });

        importsMap.insert({"extern_be_print", wasm_func_as_extern(be_print)});
    }

    auto wasm_crate1 = WasmModule::fromFile("wasm_crate1.wasm", store);
    if(!wasm_crate1.isValid())
    {
        std::cerr << "Failed to load wasm_crate1.wasm" << std::endl;
        return 1;
    }
    printf("wasm_crate1 metadata:\n");
    wasm_crate1.printMetadata();

    auto wasm_crate2 = WasmModule::fromFile("wasm_crate2.wasm", store);
    if(!wasm_crate2.isValid())
    {
        std::cerr << "Failed to load wasm_crate2.wasm" << std::endl;
        return 1;
    }
    printf("wasm_crate2 metadata:\n");
    wasm_crate2.printMetadata();

    printf("Instantiating module1\n");
    auto imports1 = wasm_crate1.buildImportList(importsMap);
    wasm_instance_t* instance1 = wasm_instance_new(store, wasm_crate1.module, &imports1, nullptr);

    if(!instance1)
    {
        wasm_message_t err;
        err.data = new wasm_byte_t[wasmer_last_error_length() + 1];
        err.size = wasmer_last_error_length();
        wasmer_last_error_message(err.data, err.size);
        std::cerr << "Failed to instantiate module1: " << err.data << std::endl;
        return 1;
    }

    printf("Instantiating module2\n");
    auto imports2 = wasm_crate2.buildImportList(importsMap);
    wasm_instance_t* instance2 = wasm_instance_new(store, wasm_crate2.module, &imports2, nullptr);

    if(!instance2)
    {
        wasm_message_t err;
        err.data = new wasm_byte_t[wasmer_last_error_length() + 1];
        err.size = wasmer_last_error_length();
        wasmer_last_error_message(err.data, err.size);
        std::cerr << "Failed to instantiate module2: " << err.data << std::endl;
        return 1;
    }

    printf("Attempting to call test_function()\n");

    wasm_extern_vec_t externs1;
    wasm_instance_exports(instance1, &externs1);
    wasm_extern_vec_t externs2;
    wasm_instance_exports(instance2, &externs2);

    {
        wasm_func_t* test_function = wasm_extern_as_func(externs1.data[wasm_crate1.exports.at("test_function").index]);
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
    }


    {
        wasm_func_t* test_extern_call_function = wasm_extern_as_func(externs1.data[wasm_crate1.exports.at("test_extern_call").index]);
        wasm_val_t empty_args_val[0] = { };
        wasm_val_t results_val[1] = {WASM_INIT_VAL};
        wasm_val_vec_t empty_args = WASM_ARRAY_VEC(empty_args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);
        if(auto t = wasm_func_call(test_extern_call_function, &empty_args, &results))
        {
            wasm_message_t msg;
            wasm_trap_message(t, &msg);
            printf("Error calling function: %s\n", msg.data);
        }

        printf("test_extern_call() returned %d\n", results_val[0].of.i32);
    }

    TestCompoennt* test_component = nullptr;
    int32_t test_component_wasm_ptr = 0;
    {
        wasm_func_t* create_test_component = wasm_extern_as_func(externs1.data[wasm_crate1.exports.at("create_test_component").index]);
        wasm_val_t args_val[] = { };
        wasm_val_t results_val[1] = {WASM_INIT_VAL};
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);
        if(auto t = wasm_func_call(create_test_component, &args, &results))
        {
            wasm_message_t msg;
            wasm_trap_message(t, &msg);
            printf("Error calling function: %s\n", msg.data);
        }

        test_component_wasm_ptr = results_val[0].of.i32;
        /*printf("create test component returned %d\n", results_val[0].of.i32);
        test_component = (TestCompoennt*)&wasm_memory_data(instance_memory)[results_val[0].of.i32];
        printf("Component initial values: %d, %d, %f\n", test_component->a, test_component->b, test_component->c);*/
    }
    {
        wasm_func_t* test_component_access = wasm_extern_as_func(externs2.data[wasm_crate2.exports.at("test_component_access").index]);
        wasm_val_t args_val[1] = { WASM_I32_VAL(test_component_wasm_ptr) };
        wasm_val_t results_val[1] = {WASM_INIT_VAL};
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);
        if(auto t = wasm_func_call(test_component_access, &args, &results))
        {
            wasm_message_t msg;
            wasm_trap_message(t, &msg);
            printf("Error calling test_component_access: %s\n", msg.data);
        }

        test_component = (TestCompoennt*)&wasm_memory_data(instance_memory)[results_val[0].of.i32];
        printf("test_component_access() returned %d\n", results_val[0].of.i32);
        printf("Component values after access: %d, %d, %f\n", test_component->a, test_component->b, test_component->c);
    }


    wasm_extern_vec_delete(&externs1);
    wasm_extern_vec_delete(&externs2);
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    return 0;
}
