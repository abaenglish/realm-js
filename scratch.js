Realm = require(".");

Cat = {
  name: "Cat",
  primaryKey: "_id",
  properties: {
    _id: "objectId",
    name: "string",
    age: "int",
    type: "string",
  },
};
Dog = {
  name: "Dog",
  primaryKey: "_id",
  properties: {
    _id: "objectId",
    name: "string",
    age: "int",
    type: "string",
  },
};

app = new Realm.App({ baseUrl: "http://localhost:9090", id: "with-db-flx-djhfg" });
user = await app.logIn(Realm.Credentials.anonymous());
realm = new Realm({
  schema: [Cat, Dog],
  sync: { user, flexible: true, _sessionStopPolicy: "immediately" },
});

subs = realm.getSubscriptions();

await subs.waitForSynchronization();

let sub;
// subs =

subs.update(() => {
  subs.add(realm.objects("Cat").filtered("age > 10"), { name: "test" });
});

subs.update(() => {
  subs.add(realm.objects("Cat").filtered("age > 15"), { name: "test2" });
});

subs.update((m) => {
  sub = m.add(realm.objects("Cat").filtered("age > 101"), { name: "test" });
});

subs.update((m) => {
  sub = m.add(realm.objects("Cat").filtered("age > 15"), { name: "test2" });
});


subs.update((m) => {
  sub = m.add(realm.objects("Cat").filtered("age > 125"), { name: "test3" });
});

let sub;
subs = subs.update((m) => {
  sub = m.add(realm.objects("Cat").filtered("age > 10"));
});

subs.waitForSynchronization(() => {
  console.log("123");
});

let sub;
q = realm.objects("Cat");
subs.update((m) => {
  sub = m.add(q, { name: "cat_sub" });
});

subs.update((m) => m.removeByName("cat_sub"));

realm.getSubscriptions().getSubscriptions();

subs.update((m) => {});

subs.update((m) => {
  m.add(realm.objects("Cat"));
  m.add(realm.objects("Cat").filtered("age > 10"));
  m.add(realm.objects("Dog"));
});

subs.update((m) => {
  m.add(realm.objects("Cat"));
});

let sub1, sub2;
subs.update((m) => {
  sub1 = m.add(realm.objects("Cat").filtered("age > 5"));
  sub2 = m.add(realm.objects("Cat").filtered("age > 10"));
});
// ---

Realm = require("./types");

PersonSchema = {
  name: "Person",
  primaryKey: "_id",
  properties: {
    _id: "objectId",
    age: "int",
    name: "string",
    friends: "Person[]",
  },
};

realm = await Realm.open({
  schema: [PersonSchema],
});

subs = realm.getSubscriptions();
let sub;
subs.update((m) => {
  sub = m.add(realm.objects("Person").filtered("age > 10"), undefined);
});

let sub1;
let sub2;
subs = realm.getSubscriptions();

subs.update(function (mutableSubs) {
  console.log(1);
  sub1 = mutableSubs.add(realm.objects("Person").filtered("age > 15"));
  console.log(2);
  sub2 = mutableSubs.add(realm.objects("Person").filtered("age > 20"));
});