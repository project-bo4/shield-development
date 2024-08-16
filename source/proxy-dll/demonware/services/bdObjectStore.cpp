#include <std_include.hpp>
#include "../services.hpp"
#include "../objects.hpp"

#include <picoproto.h>
#include <utilities/io.hpp>
#include <utilities/cryptography.hpp>
#include <component/platform.hpp>

namespace demonware
{
	bdObjectStore::bdObjectStore() : service(193, "bdObjectStore")
	{
		this->register_task(3, &bdObjectStore::listUserObjects);
		this->register_task(18, &bdObjectStore::getUserObjectCounts);

		this->register_task(8, &bdObjectStore::getPublisherObject);
		this->register_task(9, &bdObjectStore::listPublisherObjectsByCategory);

		this->register_task(6, &bdObjectStore::getUserObjectsVectorized);
		this->register_task(2, &bdObjectStore::uploadUserObject);
		this->register_task(5, &bdObjectStore::getUserObject); // Un-handled; Not used by engine at all
		this->register_task(7, &bdObjectStore::uploadUserObjectsVectorized);

		this->register_task(16, &bdObjectStore::getPublisherObjectMetadatas); // Un-handled; Not needed if user already has LPC
	}

	void bdObjectStore::getUserObject(service_server* server, byte_buffer* buffer) const
	{
		auto reply = server->create_reply(this->task_id(), 20000/*BD_OBJECTSTORE_PROXY_OBJECT_NOT_FOUND*/);
		reply->send();
	}

	void bdObjectStore::getPublisherObject(service_server* server, byte_buffer* buffer) const
	{
		auto reply = server->create_reply(this->task_id(), 20000/*BD_OBJECTSTORE_PROXY_OBJECT_NOT_FOUND*/);
		reply->send();
	}

	void bdObjectStore::listUserObjects(service_server* server, byte_buffer* buffer) const
	{
		std::string response_json = generate_user_objects_list_json();
		std::string response_buff = serialize_objectstore_structed_buffer(response_json);

		auto reply = server->create_structed_reply(this->task_id());
		reply->send(response_buff);
	}

	void bdObjectStore::getUserObjectCounts(service_server* server, byte_buffer* buffer) const
	{
		std::string response_json = generate_user_objects_count_json();
		std::string response_buff = serialize_objectstore_structed_buffer(response_json);

		auto reply = server->create_structed_reply(this->task_id());
		reply->send(response_buff);
	}

	void bdObjectStore::listPublisherObjectsByCategory(service_server* server, byte_buffer* buffer) const
	{
		std::string response_json = generate_publisher_objects_list_json("");
		std::string response_buff = serialize_objectstore_structed_buffer(response_json);

		auto reply = server->create_structed_reply(this->task_id());
		reply->send(response_buff);
	}

	void bdObjectStore::getUserObjectsVectorized(service_server* server, byte_buffer* buffer) const
	{
		std::string structed_data;
		buffer->read_struct(&structed_data);

		picoproto::Message request_buffer;
		request_buffer.ParseFromBytes(reinterpret_cast<uint8_t*>(structed_data.data()), structed_data.size());

		std::string str4 = request_buffer.GetString(4);
		picoproto::Message nested_buffer;
		nested_buffer.ParseFromBytes(reinterpret_cast<uint8_t*>(str4.data()), str4.size());

		std::vector<objectID> requested_objects_list;

		rapidjson::Document requested_objects_list_json;
		requested_objects_list_json.Parse(nested_buffer.GetString(2));
		for (rapidjson::SizeType i = 0; i < requested_objects_list_json.Size(); i++) {
			requested_objects_list.push_back({ std::format("bnet-{}", platform::bnet_get_userid())/*requested_objects_list_json[i]["owner"].GetString()*/, requested_objects_list_json[i]["name"].GetString() });
		}

		std::string response_json = deliver_user_objects_vectorized_json(requested_objects_list);
		std::string response_buff = serialize_objectstore_structed_buffer(response_json);

		auto reply = server->create_structed_reply(this->task_id());
		reply->send(response_buff);
	}

	void bdObjectStore::getPublisherObjectMetadatas(service_server* server, byte_buffer* buffer) const
	{
		auto reply = server->create_structed_reply(this->task_id());
		reply->send(""); // Un-handled
	}

	void bdObjectStore::uploadUserObject(service_server* server, byte_buffer* buffer) const
	{
		std::string structed_data;
		buffer->read_struct(&structed_data);

		picoproto::Message request_buffer;
		request_buffer.ParseFromBytes(reinterpret_cast<uint8_t*>(structed_data.data()), structed_data.size());

		std::string request = request_buffer.GetString(1);
		std::string url = request_buffer.GetString(2);
		std::string data = request_buffer.GetString(3);

		std::string file = utilities::string::split(url, '/')[8];
		std::string path = get_user_file_path(file);

		if (!utilities::io::write_file(path, data))
			logger::write(logger::LOG_TYPE_DEBUG, "[bdObjectStore::uploadUserObject] error on writing '%s'", file.data());
		else
			logger::write(logger::LOG_TYPE_DEBUG, "[bdObjectStore::uploadUserObject] saved user file '%s'", file.data());

		std::string response_json = construct_file_upload_result_json(file);
		std::string response_buff = serialize_objectstore_structed_buffer(response_json);

		auto reply = server->create_structed_reply(this->task_id());
		reply->send(response_buff);
	}

	void bdObjectStore::uploadUserObjectsVectorized(service_server* server, byte_buffer* buffer) const
	{
		std::string structed_data;
		buffer->read_struct(&structed_data);

		picoproto::Message request_buffer;
		request_buffer.ParseFromBytes(reinterpret_cast<uint8_t*>(structed_data.data()), structed_data.size());

		std::string request = request_buffer.GetString(1);
		std::string payload = request_buffer.GetString(3);

		rapidjson::Document vectorized_upload_json;
		vectorized_upload_json.Parse(payload.data());

		std::vector<std::string> uploaded_objects_list;

		const rapidjson::Value& objects = vectorized_upload_json["objects"];
		for (rapidjson::SizeType i = 0; i < objects.Size(); i++)
		{
			const rapidjson::Value& content = objects[i]["content"];
			const rapidjson::Value& name = objects[i]["metadata"]["name"];

			std::string data = utilities::cryptography::base64::decode(content.GetString());
			const auto path = std::format("{}/{}", platform::get_userdata_directory(), name.GetString());

			uploaded_objects_list.push_back(name.GetString());

			if (!utilities::io::write_file(path, data))
				logger::write(logger::LOG_TYPE_DEBUG, "[bdObjectStore::uploadUserObjectsVectorized] error on writing '%s'", name.GetString());
			else
				logger::write(logger::LOG_TYPE_DEBUG, "[bdObjectStore::uploadUserObjectsVectorized] saved user file '%s'", name.GetString());
		}

		std::string response_json = construct_vectorized_upload_list_json(uploaded_objects_list);
		std::string response_buff = serialize_objectstore_structed_buffer(response_json);

		auto reply = server->create_structed_reply(this->task_id());
		reply->send(response_buff);
	}
}

