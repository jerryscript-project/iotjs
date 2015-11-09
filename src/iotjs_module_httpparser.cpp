/* Copyright 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "iotjs_def.h"
#include "iotjs_module_httpparser.h"
#include "http_parser.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_buffer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


namespace iotjs {


#define JSETPROPERTY( container_, fname_, vname_ )                      \
  do {                                                                  \
    JObject jobj(vname_ );                                              \
    container_.SetProperty(fname_, jobj);                               \
  } while(0)


// If # of header fields == HEADER_MAX, flush header to JS side.
// This is weired : # of maximum headers in C equals to HEADER_MAX-1.
// This is because , OnHeaders cb, we increase n_fields first,
// and check whether field == HEADER_MAX.
// ex) HEADER_MAX 2 means that we can keep at most 1 header field/value
// during http parsing.
// Increse this to minimize inter JS-C call
#define HEADER_MAX 10



class HTTPParserWrap : public JObjectWrap {
public:
  explicit HTTPParserWrap(JObject& parser_, http_parser_type type)
    : JObjectWrap(parser_) {
    Initialize(type);
    parser.data = this;
  }

  ~HTTPParserWrap() {
  }

  void Initialize(http_parser_type type);

  // http-parser callbacks
  int OnMessageBegin() {
    url.MakeEmpty();
    status_msg.MakeEmpty();
    return 0;
  }

  int OnUrl(const char* at, size_t length) {
    url.Append(at, length);
    return 0;
  }

  int OnStatus(const char* at, size_t length) {
    status_msg.Append(at, length);
    return 0;
  }

  int OnHeaderField(const char* at, size_t length) {

    if (n_fields == n_values) {
      n_fields++;
      // values and fields are flushed to JS
      // before corresponding OnHeaderValue is called.
      if (n_fields == HEADER_MAX) {
        Flush(); // to JS world
        n_fields = 1;
        n_values = 0;
      }
      fields[n_fields-1].MakeEmpty();
    }
    IOTJS_ASSERT(n_fields == n_values + 1);
    fields[n_fields-1].Append(at, length);

    return 0;
  }

  int OnHeaderValue(const char* at, size_t length) {
    if (n_fields != n_values) {
      n_values++;
      values[n_values-1].MakeEmpty();
    }

    IOTJS_ASSERT(n_fields == n_values);

    values[n_values-1].Append(at, length);

    return 0;
  }

  JObject makeHeader(){
    // FIXME: This must be impl. as JS Array
    JObject jheader;
    JSETPROPERTY(jheader, "length", (n_values)*2);
    for (int i=0;i<n_values;i++) {
      char index_string0 [5];
      char index_string1 [5];
      sprintf(index_string0,"%d",i*2);
      sprintf(index_string1,"%d",i*2+1);
      JObject v(values[i].data());
      JObject f(fields[i].data());
      jheader.SetProperty(index_string0, f);
      jheader.SetProperty(index_string1, v);

    }
    return jheader;
  }


  int OnHeadersComplete() {
    JObject jobj = jobject();
    JObject func = jobj.GetProperty("OnHeadersComplete");
    IOTJS_ASSERT(func.IsFunction());

    // URL
    JArgList argv(1);
    JObject info;

    if (flushed) {
      // If some headers already are flushed,
      // flush the remaining headers.
      // In Flush function, url is already flushed to JS.
      Flush();
    }
    else {
      // Here, there was no flushed header.
      // We need to make a new header object with all header fields
      JSETPROPERTY(info, "headers", makeHeader());
      if ( parser.type == HTTP_REQUEST) {
        IOTJS_ASSERT(!url.IsEmpty());
        JSETPROPERTY(info, "url", url.data());
      }
    }
    n_fields = n_values = 0;

    // Method
    if (parser.type == HTTP_REQUEST) {
      JSETPROPERTY(info, "method", (int32_t)parser.method);
    }

    // Status
    if (parser.type == HTTP_RESPONSE) {
      JSETPROPERTY(info, "status", (int32_t)parser.status_code);
      JSETPROPERTY(info, "status_msg", status_msg.data());
    }


    // For future support, current http_server module does not support
    // upgrade and keepalive.
    // upgrade
    JSETPROPERTY(info, "upgrade", parser.upgrade ? true : false);
    // shouldkeepalive
    JSETPROPERTY(info, "shouldkeepalive",
                 http_should_keep_alive(&parser) ? true : false);


    argv.Add(info);

    return (MakeCallback(func, jobj, argv).GetBoolean()? 1 : 0);

  }

  int OnBody(const char* at, size_t length) {
    JObject jobj = jobject();
    JObject func = jobj.GetProperty("OnBody");
    IOTJS_ASSERT(func.IsFunction());

    JArgList argv(3);
    argv.Add(*cur_jbuf);
    int offset = (int)(at-cur_buf);
    JObject start(offset);
    argv.Add(start);
    JObject leng((int)length);
    argv.Add(leng);


    MakeCallback(func, jobj, argv);

    return 0;
  }

  int OnMessageComplete() {
    JObject jobj = jobject();
    JObject func = jobj.GetProperty("OnMessageComplete");
    IOTJS_ASSERT(func.IsFunction());

    MakeCallback(func, jobj, JArgList::Empty());

    return 0;
  }

  void Flush() {
    JObject jobj = jobject();
    JObject func = jobj.GetProperty("OnHeaders");
    IOTJS_ASSERT(func.IsFunction());


    JArgList argv(2);
    JObject jheader(makeHeader());
    argv.Add(jheader);
    if (parser.type == HTTP_REQUEST && !url.IsEmpty()) {
      JObject jurl(url.data());
      argv.Add(jurl);
    }

    MakeCallback(func, jobj, argv);

    url.MakeEmpty();
    flushed = true;
  }

  http_parser parser;
  String url;
  String status_msg;
  String fields[HEADER_MAX];
  String values[HEADER_MAX];
  int n_fields;
  int n_values;
  JObject* cur_jbuf;
  char* cur_buf;
  int cur_buf_len;
  bool flushed;
};


#define HTTP_CB(name)                                                   \
  int name(http_parser* parser) {                                       \
    HTTPParserWrap* container = (HTTPParserWrap*)(parser->data);        \
    return container->name();                                           \
  }                                                                     \


#define HTTP_DATA_CB(name)                                              \
  int name(http_parser* parser, const char* at, size_t length) {        \
    HTTPParserWrap* container = (HTTPParserWrap*)(parser->data);        \
    return container->name(at, length);                                 \
  }                                                                     \



//http-parser callbacks
HTTP_CB( OnMessageBegin );
HTTP_DATA_CB( OnUrl );
HTTP_DATA_CB( OnStatus );
HTTP_DATA_CB( OnHeaderField );
HTTP_DATA_CB( OnHeaderValue );
HTTP_CB( OnHeadersComplete );
HTTP_DATA_CB( OnBody );
HTTP_CB( OnMessageComplete );


const struct http_parser_settings settings = {
  iotjs::OnMessageBegin,
  iotjs::OnUrl,
  iotjs::OnStatus,
  iotjs::OnHeaderField,
  iotjs::OnHeaderValue,
  iotjs::OnHeadersComplete,
  iotjs::OnBody,
  iotjs::OnMessageComplete,
};


void HTTPParserWrap::Initialize(http_parser_type type) {
  http_parser_init(&parser, type);
  url.MakeEmpty();
  status_msg.MakeEmpty();
  n_fields = 0;
  n_values = 0;
  flushed = false;
  cur_jbuf = NULL;
  cur_buf = NULL;
  cur_buf_len = 0;
}


JHANDLER_FUNCTION(Reinitialize) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());

  JObject* jparser = handler.GetThis();

  http_parser_type httpparser_type =
    static_cast<http_parser_type>(handler.GetArg(0)->GetInt32());
  IOTJS_ASSERT(httpparser_type == HTTP_REQUEST ||
               httpparser_type == HTTP_RESPONSE);

  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());
  parser->Initialize(httpparser_type);

  return true;
}


JHANDLER_FUNCTION(Finish) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 0);

  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());

  int rv = http_parser_execute(&(parser->parser), &settings, NULL, 0);

  if (rv != 0) {
    enum http_errno err = HTTP_PARSER_ERRNO(&parser->parser);

    JObject eobj(JObject::Error("Parse Error"));
    JSETPROPERTY(eobj, "byteParsed", 0);
    JSETPROPERTY(eobj, "code", http_errno_name(err));
    handler.Return(eobj);
  }

  return true;
}


JHANDLER_FUNCTION(Execute) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsObject());

  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());


  JObject* jbuffer = handler.GetArg(0);
  BufferWrap* buffer = BufferWrap::FromJBuffer(*jbuffer);
  char* buf_data = buffer->buffer();
  int buf_len = buffer->length();
  JHANDLER_CHECK(buf_data != NULL);
  JHANDLER_CHECK(buf_len > 0);

  parser->cur_jbuf = jbuffer;
  parser->cur_buf = buf_data;
  parser->cur_buf_len = buf_len;

  int nparsed = http_parser_execute(&(parser->parser), &settings,
                                    buf_data, buf_len);

  parser->cur_jbuf = NULL;
  parser->cur_buf = NULL;
  parser->cur_buf_len = 0;


  if (!parser->parser.upgrade && nparsed != buf_len) {
    // nparsed should equal to buf_len except UPGRADE protocol
    enum http_errno err = HTTP_PARSER_ERRNO(&parser->parser);
    JObject eobj(JObject::Error("Parse Error"));
    JSETPROPERTY(eobj, "byteParsed", 0);
    JSETPROPERTY(eobj, "code", http_errno_name(err));
    handler.Return(eobj);
  }
  else{
    JObject ret(nparsed);
    handler.Return(ret);
  }

  return true;
}


JHANDLER_FUNCTION(Pause) {
  JHANDLER_CHECK(handler.GetArgLength() == 0);
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());
  http_parser_pause(&parser->parser, 1);
  return true;
}


JHANDLER_FUNCTION(Resume) {
  JHANDLER_CHECK(handler.GetArgLength() == 0);
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());
  http_parser_pause(&parser->parser, 0);
  return true;
}


JHANDLER_FUNCTION(HTTPParserCons) {
  JHANDLER_CHECK(handler.GetThis()->IsObject());
  JHANDLER_CHECK(handler.GetArgLength() == 1);
  JHANDLER_CHECK(handler.GetArg(0)->IsNumber());

  JObject* jparser = handler.GetThis();

  http_parser_type httpparser_type =
    static_cast<http_parser_type>(handler.GetArg(0)->GetInt32());
  IOTJS_ASSERT(httpparser_type == HTTP_REQUEST ||
               httpparser_type == HTTP_RESPONSE);
  HTTPParserWrap* httpparser_wrap = new HTTPParserWrap(*jparser,
                                                       httpparser_type);
  IOTJS_ASSERT(httpparser_wrap->jobject().IsObject());
  IOTJS_ASSERT(reinterpret_cast<HTTPParserWrap*>(jparser->GetNative())
         == httpparser_wrap);
  return true;
}


JObject* InitHttpparser() {

  Module* module = GetBuiltinModule(MODULE_HTTPPARSER);
  JObject* httpparser = module->module;

  if (httpparser == NULL) {
    httpparser = new JObject();

    JObject HTTPParserConstructor(HTTPParserCons);
    httpparser->SetProperty("HTTPParser", HTTPParserConstructor);

    JObject request(HTTP_REQUEST);
    HTTPParserConstructor.SetProperty("REQUEST", request);
    JObject response(HTTP_RESPONSE);
    HTTPParserConstructor.SetProperty("RESPONSE", response);

    JObject methods;
#define V(num, name, string)                                                  \
    JSETPROPERTY(methods, #num, #string);
  HTTP_METHOD_MAP(V)
#undef V

    JSETPROPERTY(HTTPParserConstructor, "methods", methods);

    JObject prototype;
    HTTPParserConstructor.SetProperty("prototype", prototype);
    // prototype ...
    prototype.SetMethod("execute", Execute);
    prototype.SetMethod("reinitialize", Reinitialize);
    prototype.SetMethod("finish", Finish);
    prototype.SetMethod("pause", Pause);
    prototype.SetMethod("resume", Resume);

    module->module = httpparser;
  }

  return httpparser;

}


} // namespace iotjs
