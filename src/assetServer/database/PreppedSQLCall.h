//
// Created by eli on 5/20/2022.
//

#ifndef BRANEENGINE_PREPPEDSQLCALL_H
#define BRANEENGINE_PREPPEDSQLCALL_H
#include <sqlite/sqlite3.h>
#include <stdexcept>
#include <functional>
#include <tuple>
#include <string>

template<typename... Args>
class PreppedSQLCall
{
	sqlite3_stmt* stmt = nullptr;

	void checkBindArg(int res)
	{
		if(res != SQLITE_OK)
			throw std::runtime_error("could not bind argument, returned with result: " + std::to_string(res));
	}

	template<typename... Types>
	void bindArgs(Types... args)
	{
		bindArgs(1, args...);
	}

	template<typename... Types>
	void bindArgs(int index, int32_t arg, Types... args)
	{
		checkBindArg(sqlite3_bind_int(stmt, index, arg));
		if constexpr(sizeof...(Types))
			bindArgs(index + 1, &args...);
	}

	template<typename... Types>
	void bindArgs(int index, int64_t arg, Types... args)
	{
		checkBindArg(sqlite3_bind_int64(stmt, index, arg));
		if constexpr(sizeof...(Types))
			bindArgs(index + 1, &args...);
	}

	template<typename... Types>
	void bindArgs(int index, double arg, Types... args)
	{
		checkBindArg(sqlite3_bind_double(stmt, index, arg));
		if constexpr(sizeof...(Types))
			bindArgs(index + 1, &args...);
	}

	template<typename... Types>
	void bindArgs(int index, const std::string& arg, Types... args)
	{
		checkBindArg(sqlite3_bind_text(stmt, index, arg.data(), arg.size(), SQLITE_TRANSIENT));
		if constexpr(sizeof...(Types))
			bindArgs(index + 1, &args...);
	}

	template<typename... Types>
	void getColumns(std::tuple<Types...>& columns)
	{
		getColumns<0, Types...>(columns);
	}

	template<size_t index, typename... Types>
	void getColumns(std::tuple<Types...>& columns)
	{
		getColumn(std::get<index>(columns), index);

		if constexpr(index + 1 < sizeof...(Types))
			getColumns<index + 1, Types...>(columns);
	}

	void getColumn(int32_t & value, int index)
	{
		value = sqlite3_column_int(stmt, index);
	}

	void getColumn(int64_t& value, int index)
	{
		value = sqlite3_column_int64(stmt, index);
	}

	void getColumn(double& value, int index)
	{
		value = sqlite3_column_double(stmt, index);
	}

	void getColumn(std::string& value, int index)
	{
		const char* data = (const char*)sqlite3_column_text(stmt, index);
		size_t size = sqlite3_column_bytes(stmt, index);
		value = std::string(data, size);
	}

public:

	void initialize(const std::string& sql, sqlite3* db)
	{
		int res = sqlite3_prepare_v2(db, sql.data(), sql.size(), &stmt, nullptr);
		if(res != SQLITE_OK)
		{
			throw std::runtime_error("Could not initialize prepared sql call: " + sql + "\nsqlite returned: " + std::to_string(res));
		}
	}

	~PreppedSQLCall()
	{
		if(stmt)
			sqlite3_finalize(stmt);
	}

	template<typename... Columns>
	void run(Args... args, std::function<void(Columns... )> f)
	{
		assert(stmt);
		bindArgs(args...);
		int result = SQLITE_ROW;
		while(result == SQLITE_ROW)
		{
			result = sqlite3_step(stmt);
			if(result == SQLITE_ROW)
			{
				std::tuple<Columns...> columns;
				getColumns(columns);
				std::apply(f, columns);
			}
		}
		if(result != SQLITE_DONE)
			throw std::runtime_error("Prepared SQL statement failed with return value: " + std::to_string(result));
		sqlite3_reset(stmt);
	}
};


#endif //BRANEENGINE_PREPPEDSQLCALL_H
