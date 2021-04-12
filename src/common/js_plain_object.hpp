////////////////////////////////////////////////////////////////////////////
//
// Copyright 2021 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#ifndef JS_DICTIONARY_HEADER
#define JS_DICTIONARY_HEADER

#include <iostream>

#include "object/interfaces.hpp"
#include "object/strategies.hpp"

#pragma once

#if REALM_ANDROID
#include <android/log.h>
#endif

namespace realm {
namespace js {

template <
        typename VM,
        typename GetterSetters,
        typename Methods = NoMethods<VM>,
        typename Collection = NoData>
struct JSObject : public ObjectObserver {
   private:
    using Value = js::Value<VM>;
    using Object = js::Object<VM>;
    using ObjectType = typename VM::Object;
    using ContextType = typename VM::Context;

    ContextType context;
    bool waiting_for_notifications{false};

    std::unique_ptr<Methods> methods;
    std::unique_ptr<GetterSetters> getters_setters;
    std::unique_ptr<Collection> collection;
    std::vector<Subscriber*> subscribers;
    common::JavascriptObject js_object;

   public:
    template <typename RealmData>
    JSObject(ContextType _context, RealmData _data)
        : context{_context}, js_object{_context} {
        getters_setters = std::make_unique<GetterSetters>();
        methods = std::make_unique<Methods>();
        collection = std::make_unique<Collection>(_data);

        collection->on_change([=](collection::Notification notification) {
            std::cout << "Update! \n";
            update(notification);

            if(notification.from_realm){
                notify_subscriber(notification);
            }
        });
    };

    Collection* get_collection() { return collection.get(); }

    void watch_collection() {
        if (waiting_for_notifications) {
            return;
        }

        waiting_for_notifications = collection->watch();
    }

    void subscribe(Subscriber* subscriber) {
        subscribers.push_back(subscriber);
        watch_collection();
    }

    void remove_subscription(const Subscriber* subscriber) {
        int index = -1;
        for (auto const& candidate : subscribers) {
            index++;
            if (candidate->equals(subscriber)) {
                subscribers.erase(subscribers.begin() + index);
                break;
            }
        }
    }

    void unsubscribe_all() { subscribers.clear(); }

    void notify_subscriber(collection::Notification notification){
        HANDLESCOPE(context)
        for (Subscriber* subscriber : subscribers) {
            subscriber->notify(js_object.get_object(), notification.change_set);
        }
    }

    template <typename Realm_ChangeSet>
    void update(Realm_ChangeSet& change_set) {
        /* This is necessary for NodeJS. */
        std::cout << "Update! \n";
        HANDLESCOPE(context)
        getters_setters->update(js_object, this);
    }

    ObjectType build() {
        methods->apply(js_object, this);
        getters_setters->apply(js_object, this);

#if REALM_ANDROID
        js_object.set_collection(collection.get());
        js_object.set_observer(this);
#endif
        return js_object.get_object();
    }

    template <typename CB>
    void setup_finalizer(ObjectType object, CB&& cb) {
        // This method gets called when GC dispose the JS Object.
        common::JavascriptObject::finalize(
            object, [=]() { cb(); }, this);
    }

    ~JSObject() = default;
};
}  // namespace js
}  // namespace realm

#endif