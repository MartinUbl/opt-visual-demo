/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>

/**
 * Base interface for all objects managed by CObjectAccessor
 */
class IObject {
	public:
		virtual ~IObject() = default;
};

/**
 * Global object accessor and manager (singleton pattern)
 * Ensures only one instance of each object type exists
 */
class CObjectAccessor {
	public:
		CObjectAccessor() = delete;
		~CObjectAccessor() = delete;

		template <typename T>
		static T& get() {
			static_assert(std::is_base_of<IObject, T>::value, "T must inherit from IObject");

			auto typeId = std::type_index(typeid(T));
			auto it = s_instances.find(typeId);

			if (it == s_instances.end()) {
				auto instance = std::make_unique<T>();
				T& ref = *instance;
				s_instances[typeId] = std::move(instance);
				return ref;
			}

			return *static_cast<T*>(it->second.get());
		}

	private:
		inline static std::unordered_map<std::type_index, std::unique_ptr<IObject>> s_instances;
};
