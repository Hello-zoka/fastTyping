#include "server.h"
#include <boost/log/trivial.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <thread>
#include "game.h"

namespace FastTyping::Server {
Server::Server()
    : acceptor(ioContext, tcp::endpoint(tcp::v4(), PORT)),
      userStorage(new Database),
      gameStorage(new MapGameStorage) {
    std::cout << "Listening at " << acceptor.local_endpoint() << std::endl;
    commonQueriesMap["echo"] = [&](const json &body, User &user) {
        return body;
    };
    commonQueriesMap["createGame"] = [&](const json &body, User &user) -> json {
        // basic checks
        if (!body.contains("dictionaryName") ||
            !body["dictionaryName"].is_string()) {
            return {{"header", {{"type", "wrongFormatError"}}},
                    {"body", {{"text", "can't find \"dictionaryName\""}}}};
        }

        if (!body.contains("parserName") || !body["parserName"].is_string()) {
            return {{"header", {{"type", "wrongFormatError"}}},
                    {"body", {{"text", "can't find \"parserName\""}}}};
        }

        if (user.getGame() != nullptr) {
            return {{"header", {{"type", "alreadyInGameError"}}},
                    {"body", {{"text", "Already in game"}}}};
        }

        auto result = gameStorage->createGame(body);
        if (body.contains("autoJoin") && body["autoJoin"].is_boolean() &&
            body["autoJoin"]) {
            if (!result["body"].contains("id") ||
                !result["body"]["id"].is_number()) {
                result["body"]["joined"] = false;
                return result;
            }
            json errors;
            auto game = gameStorage->get(result["body"]["id"], errors);
            if (game == nullptr) {
                result["body"]["joined"] = false;
                result["body"]["error"] = errors;
            } else {
                user.setGame(game);
                result["body"]["joined"] = true;
                result["body"]["error"] = "";
            }
        }
        return result;
    };
    commonQueriesMap["joinGame"] = [&](const json &body, User &user) -> json {
        if (!body.contains("id") || !body["id"].is_number_unsigned()) {
            return {{"header", {{"type", "wrongFormatError"}}},
                    {"body", {{"text", "can't find \"id\""}}}};
        }

        if (user.getGame() != nullptr) {
            return {{"header", {{"type", "alreadyInGameError"}}},
                    {"body", {{"text", "Already in game"}}}};
        }
        json errors;
        auto game = gameStorage->get(body["id"], errors);
        if (game == nullptr) {
            return errors;
        }
        if (game->hasUser(user.getId())) {
            return {{"header", {{"type", "connectAfterLeaveError"}}},
                    {"body", {{"text", "Try connect after disconnect"}}}};
        }
        user.setGame(game);
        return {{"header", {{"type", "GameJoinedSuccessfully"}}},
                {"body", {{"id", game->getId()}}}};
    };
    commonQueriesMap["leaveGame"] = [&](const json &body, User &user) -> json {
        if (user.getGame() == nullptr) {
            return {{"header", {{"type", "notInGameError"}}},
                    {"body", {{"text", "Not in game"}}}};
        }

        user.setGame(nullptr);
        return {{"header", {{"type", "GameLeaveSuccessfully"}}}, {"body", {}}};
    };
    commonQueriesMap["getNewLine"] = [&](const json &body, User &user) -> json {
        if (user.getGame() == nullptr) {
            return {{"header", {{"type", "notInGameError"}}},
                    {"body", {{"text", "not in game"}}}};
        }
        return user.getGame()->getNewLine(user.getId());
    };
    commonQueriesMap["addNewChar"] = [&](const json &body, User &user) -> json {
        if (user.getGame() == nullptr) {
            return {{"header", {{"type", "notInGameError"}}},
                    {"body", {{"text", "not in game"}}}};
        }
        if (!body.contains("char") || !body["char"].is_string() ||
            body["char"].get<std::string>().size() != 1) {
            return {{"header", {{"type", "wrongFormatError"}}},
                    {"body", {{"text", "can't find word to check"}}}};
        }

        auto result = user.getGame()->addNewChar(
            user.getId(), body["char"].get<std::string>()[0]);
        return result;
    };
    commonQueriesMap["backspace"] = [&](const json &body, User &user) -> json {
        if (user.getGame() == nullptr) {
            return {{"header", {{"type", "notInGameError"}}},
                    {"body", {{"text", "not in game"}}}};
        }
        return user.getGame()->backspace(user.getId());
    };
    commonQueriesMap["getStates"] = [&](const json &body, User &user) -> json {
        if (user.getGame() == nullptr) {
            return {{"header", {{"type", "notInGameError"}}},
                    {"body", {{"text", "not in game"}}}};
        }
        return user.getGame()->getStateOfUsers();
    };
    commonQueriesMap["exit"] = [&](const json &body, User &user) -> json {
        user.setWillToExit();
        return {};
    };


    loginQueriesMap["login"] = [&](const json &body) -> json {
          // basic checks
          if (!body.contains("name") ||
              !body["name"].is_string()) {
              return {{"header", {{"type", "wrongFormatError"}}},
                      {"body", {{"text", "can't find \"name\""}}}};
          }
          std::string name = body["name"];
          if (!body.contains("password") || !body["password"].is_string()) {
              return {{"header", {{"type", "wrongFormatError"}}},
                      {"body", {{"text", "can't find \"password\""}}}};
          }
          std::string password = body["password"];
          auto result = userStorage->login(name, password);
          return result;
    };
    loginQueriesMap["register"] = [&](const json &body) -> json {
      // basic checks
      if (!body.contains("name") ||
          !body["name"].is_string()) {
          return {{"header", {{"type", "wrongFormatError"}}},
                  {"body", {{"text", "can't find \"name\""}}}};
      }
      std::string name = body["name"];
      if (!body.contains("password") || !body["password"].is_string()) {
          return {{"header", {{"type", "wrongFormatError"}}},
                  {"body", {{"text", "can't find \"password\""}}}};
      }
      std::string password = body["password"];
      auto result = userStorage->registration(name, password);
      return result;
    };
    loginQueriesMap["changePassword"] = [&](const json &body) -> json {
      // basic checks
      if (!body.contains("name") ||
          !body["name"].is_string()) {
          return {{"header", {{"type", "wrongFormatError"}}},
                  {"body", {{"text", "can't find \"name\""}}}};
      }
      std::string name = body["name"];
      if (!body.contains("old_password") || !body["old_password"].is_string()) {
          return {{"header", {{"type", "wrongFormatError"}}},
                  {"body", {{"text", "can't find \"old password\""}}}};
      }
      std::string old_password = body["old_password"];
      if (!body.contains("new_password") || !body["new_password"].is_string()) {
          return {{"header", {{"type", "wrongFormatError"}}},
                  {"body", {{"text", "can't find \"new password\""}}}};
      }
      std::string new_password = body["new_password"];
      auto result = userStorage->changePassword(name, old_password, new_password);
      return result;
    };
}

[[noreturn]] void Server::polling() {
    while (true) {
        tcp::socket sock = acceptor.accept();
        std::thread([s = std::move(sock), this]() mutable {
            parseQuery(std::move(s));
        }).detach();
    }
}

void Server::parseQuery(tcp::socket s) {
    try {
        tcp::iostream client(std::move(s));
        std::cout << "New client: " << client.socket().remote_endpoint() << "->"
                  << client.socket().local_endpoint() << std::endl;
        std::string line;
        json query, result;
        std::string user_name;
        std::optional<json> errors;
        try {
            while (client) {
                if (!std::getline(client, line)) {
                    std::cout << "Disconnected: " << client.socket().remote_endpoint()
                              << "->" << client.socket().local_endpoint() << std::endl;
                    return;
                }
                errors = checkQueryCorrectness(line);
                if (errors) {
                    client << errors.value() << '\n';
                    continue;
                }
                query = json::parse(line);
                auto queryHeader = query["header"];
                auto queryBody = query["body"];
                BOOST_LOG_TRIVIAL(debug) << query << '\n';
                if (auto it = loginQueriesMap.find(queryHeader["type"]);
                    it != loginQueriesMap.end()) {
                    result = it->second(queryBody);
                    result["header"]["queryType"] = queryHeader["type"];
                    client << result << '\n';
                    if ((result["header"]["queryType"] == "login" || result["header"]["queryType"] == "register") && result["header"]["type"] == "success") {
                        user_name = queryBody["name"].get<std::string>();
                        break;
                    }
                }
            }
        } catch (nlohmann::detail::exception &e) {
            std::cerr << e.what() << std::endl;  // process error to client
        }
      
        // Case 1: login:  if( userStorage->nameExist(name) == true &&
        // passw == getPassword(userStorage->getId(name)) ) go next,
        // else send error to UI Case 2: register: if
        // (userStorage->nameExist(name) == false) {
        // setPassword(userStorage->getId(name))} Case 3: change
        // password: match as in login and then setPassword

        // That's not user's methods because it's hard to hold user in
        // incorrect password cases etc

        User user(
            user_name,
            userStorage.get());  // after you parse login and register query
                                 // you can simply identify user by its name
//        result = {{"header", {{"type", "loginSuccessfully"}}},
//                       {"body", {{"name", user.name()}}}};
//        client << result << '\n';

        try {
            while (client) {
                if (!std::getline(client, line)) {
                    break;
                }
                errors = checkQueryCorrectness(line);
                if (errors) {
                    client << errors.value() << '\n';
                    continue;
                }
                query = json::parse(line);
                auto queryHeader = query["header"];
                auto queryBody = query["body"];
                BOOST_LOG_TRIVIAL(debug) << query << '\n';
                if (auto it = commonQueriesMap.find(queryHeader["type"]);
                    it != commonQueriesMap.end()) {
                    result = it->second(queryBody, user);
                    result["header"]["queryType"] = queryHeader["type"];
                    if (user.isWantToExit()) {
                        user.clearWillToExit();
                        break;
                    }
                    client << result << '\n';
                }
            }
        } catch (nlohmann::detail::exception &e) {
            std::cerr << e.what() << std::endl;  // process error to client
        }
        user.setGame(nullptr);
        std::cout << "Disconnected: " << client.socket().remote_endpoint()
                  << "->" << client.socket().local_endpoint() << std::endl;
    } catch (boost::system::system_error &e) {
        std::cerr << "Unknown Boost::ASIO error occurred: " << e.what() << '\n';
    }
}
std::optional<json> Server::checkQueryCorrectness(
    const std::string &queryString) {
    json query;
    try {
        query = json::parse(queryString);
    } catch (nlohmann::detail::exception &e) {
        return {{{"header", {{"type", "wrongFormatError"}}},
                 {"body", {{"text", "bad json"}}}}};
    }
    if (!query.contains("header") || !query["header"].is_object()) {
        return {{{"header", {{"type", "wrongFormatError"}}},
                 {"body", {{"text", "can't find header"}}}}};
    }
    if (!query.contains("body") ||
        (!query["body"].is_null() && !query["body"].is_object())) {
        return {{{"header", {{"type", "wrongFormatError"}}},
                 {"body", {{"text", "can't find body"}}}}};
    }

    if (!query["header"].contains("type") ||
        !query["header"]["type"].is_string()) {
        return {{{"header", {{"type", "wrongFormatError"}}},
                 {"body", {{"text", "can't recognize type of query"}}}}};
    }

    return {};
}
}  // namespace FastTyping::Server