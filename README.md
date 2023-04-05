c++实现的json解析和生成器

利用c++11的初始化来构造JsonObject 和 JsonArray对象

```
Json::object json = Json::object{
  {"key", "value"}
};

string = json.dump();
```
