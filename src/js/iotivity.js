/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

var iotivityBuiltin = process.binding(process.binding.iotivity);
var util = require('util');

function Iotivity() {
}

function propertyPropertiesNames(values) {
  var names = Object.getOwnPropertyNames(values);
  values.names = names;
  var index;
  for (index in names) {
    if (util.isObject(values[names[index]]) &&
      !util.isArray(values[names[index]])) {
      propertyPropertiesNames(values[names[index]]);
    }
    if (util.isArray(values[names[index]])) {
      propertyPropertiesNames(values[names[index]]);
    }
  }
}

Iotivity.prototype.OCInit = function(addr, port, mode) {
  if (util.isNull(addr)) {
    var newAddr = "";
    return iotivityBuiltin.OCInit(newAddr, port, mode);
  }

  if (!util.isString(addr)) {
    console.error("Wrong address in OCInit");

    return 255;
  }

  if (!util.isUndefined(addr) && util.isString(addr)) {
    return iotivityBuiltin.OCInit(addr, port, mode);
  }
};

Iotivity.prototype.OCProcess = function() {
  return iotivityBuiltin.OCProcess();
};

Iotivity.prototype.OCSetDeviceInfo = function(OCDeviceInfo) {
  return iotivityBuiltin.OCSetDeviceInfo(OCDeviceInfo);
};

Iotivity.prototype.OCSetPlatformInfo = function(OCPlatformInfo) {
  return iotivityBuiltin.OCSetPlatformInfo(OCPlatformInfo);
};

Iotivity.prototype.OCStop = function() {
  return iotivityBuiltin.OCStop();
};

Iotivity.prototype.OCCreateResource = function(handle, typeName,
                                                interfaceName, uri, callback,
                                                resourceProperties) {
  resources = {};
  resources.typeName = typeName;
  resources.interfaceName = interfaceName;
  resources.uri = uri;

  var callbackObject = {};
  callbackObject.callback = callback;

  return iotivityBuiltin.OCCreateResource(handle, resources, callbackObject,
                                           resourceProperties);
};

Iotivity.prototype.OCDeleteResource = function(handle) {
  return iotivityBuiltin.OCDeleteResource(handle);
};

Iotivity.prototype.OCDoResource = function(handle, method, requestUri,
  destination, payload, connType,
  qos, callback, options, number) {
  doResources = {};
  doResources.method = method;
  doResources.requestUri = requestUri;
  doResources.destination = destination;
  doResources.connType = connType;
  doResources.qos = qos;
  var callbackObject = {};
  callbackObject.callback = callback;

  if (util.isNull(number)) {
    doResources.number = 0;
  }
  else {
    doResources.number = number;
  }

  if (util.isNull(payload)) {
    payload = {};
  }
  else {
    if (!util.isNullOrUndefined(payload.values)) {
      propertyPropertiesNames(payload.values);
    }
    if (!util.isNullOrUndefined(payload.next)) {
      propertyPropertiesNames(payload.next);
    }
  }

  if (util.isNull(destination)) {
    destination = {};
  }

  if (util.isNull(options)) {
    options = [];
  }

  return iotivityBuiltin.OCDoResource(handle, doResources, payload,
                                       callbackObject, options);
};

Iotivity.prototype.OCCancel = function(handle, qos, options) {
  return iotivityBuiltin.OCCancel(handle, qos, options);
};

Iotivity.prototype.OCGetServerInstanceIDString = function() {
  return iotivityBuiltin.OCGetServerInstanceIDString();
};

Iotivity.prototype.OCGetNumberOfResources = function(numberOfResources) {
  return iotivityBuiltin.OCGetNumberOfResources(numberOfResources);
};

Iotivity.prototype.OCDoResponse = function(response) {
  if (util.isNull(response.payload)) {
    response.payload = {};
  }
  if (!util.isNullOrUndefined(response.payload)) {
    if (!util.isNullOrUndefined(response.payload.values)) {
      propertyPropertiesNames(response.payload.values);
    }
    if (!util.isNullOrUndefined(response.payload.next)) {
      propertyPropertiesNames(response.payload.next);
    }
  }

  return iotivityBuiltin.OCDoResponse(response);
};

Iotivity.prototype.OCNotifyAllObservers = function(handle, qos) {
  return iotivityBuiltin.OCNotifyAllObservers(handle, qos);
};

Iotivity.prototype.OCNotifyListOfObservers = function(handle, obsIdList,
                                                      payload, qos) {
  if (!util.isNullOrUndefined(payload)) {
    if (!util.isNullOrUndefined(payload.values)) {
      var names = Object.getOwnPropertyNames(payload.values);
      payload.values.names = names;
    }
  }

  return iotivityBuiltin.OCNotifyListOfObservers(handle, obsIdList, payload,
                                                  qos);
};

Iotivity.prototype.OCStartPresence = function(ttl) {
  return iotivityBuiltin.OCStartPresence(ttl);
};

Iotivity.prototype.OCStopPresence = function() {
  return iotivityBuiltin.OCStopPresence();
};

Iotivity.prototype.OCGetNumberOfResourceInterfaces = function(handle, number) {
  return iotivityBuiltin.OCGetNumberOfResourceInterfaces(handle, number);
};

Iotivity.prototype.OCGetNumberOfResourceTypes = function(handle, number) {
  return iotivityBuiltin.OCGetNumberOfResourceTypes(handle, number);
};

Iotivity.prototype.OCGetResourceHandle = function(index) {
  return iotivityBuiltin.OCGetResourceHandle(index);
};

Iotivity.prototype.OCGetResourceHandleFromCollection = function(handle,index) {
  return iotivityBuiltin.OCGetResourceHandleFromColl(handle, index);
};

Iotivity.prototype.OCGetResourceInterfaceName = function(handle, index) {
  return iotivityBuiltin.OCGetResourceInterfaceName(handle, index);
};

Iotivity.prototype.OCGetResourceTypeName = function(handle, index) {
  return iotivityBuiltin.OCGetResourceTypeName(handle, index);
};

Iotivity.prototype.OCGetResourceProperties = function(handle) {
  return iotivityBuiltin.OCGetResourceProperties(handle);
};

Iotivity.prototype.OCGetResourceUri = function(handle) {
  return iotivityBuiltin.OCGetResourceUri(handle);
};

Iotivity.prototype.OCGetResourceHandler = function(handle) {
  return iotivityBuiltin.OCGetResourceHandler(handle);
};

Iotivity.prototype.OCBindResourceHandler = function(handle, callback) {
  var callbackObject = {};
  callbackObject.callback = callback;
  return iotivityBuiltin.OCBindResourceHandler(handle, callbackObject);
};

Iotivity.prototype.OCBindResourceInterfaceToResource = function(handle,
    resourceInterfaceName) {
  return iotivityBuiltin.OCBindResourceInterfaceToRes(handle,
      resourceInterfaceName);
};

Iotivity.prototype.OCBindResourceTypeToResource = function(handle, typeName) {
  return iotivityBuiltin.OCBindResourceTypeToResource(handle, typeName);
};

Iotivity.prototype.OCBindResource = function(collectionHandle,resourceHandle) {
  return iotivityBuiltin.OCBindResource(collectionHandle, resourceHandle);
};

Iotivity.prototype.OCUnBindResource = function(collectionHandle,
                                               resourceHandle) {
  return iotivityBuiltin.OCUnBindResource(collectionHandle, resourceHandle);
};

Iotivity.prototype.OCSetDefaultDeviceEntityHandler = function(callback) {
  var callbackObject = {};
  callbackObject.callback = callback;
  return iotivityBuiltin.OCSetDefaultDeviceEntityHandler(callbackObject);
};

Iotivity.prototype.OCRegisterPersistentStorageHandler = function(callbacks) {
  return iotivityBuiltin.OCRegisterPS(callbacks);
};

var WITH_PRESENCE = true;

/**
 *  Default discovery mechanism using '/oic/res' is supported by
 *  all OIC devices That are Discoverable.
 */
Iotivity.prototype.OC_RSRVD_WELL_KNOWN_URI = "/oic/res";

/** Device URI.*/
Iotivity.prototype.OC_RSRVD_DEVICE_URI = "/oic/d";

/** Platform URI.*/
Iotivity.prototype.OC_RSRVD_PLATFORM_URI = "/oic/p";

/** Resource Type.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPES_URI = "/oic/res/types/d";

/** Gateway URI.*/
Iotivity.prototype.OC_RSRVD_GATEWAY_URI = "/oic/gateway";

/** MQ Broker URI.*/
Iotivity.prototype.OC_RSRVD_WELL_KNOWN_MQ_URI = "/oic/ps";

/** KeepAlive URI.*/
Iotivity.prototype.OC_RSRVD_KEEPALIVE_URI = "/oic/ping";


/** Presence */

/** Presence URI through which the OIC devices advertise their presence.*/
Iotivity.prototype.OC_RSRVD_PRESENCE_URI = "/oic/ad";

/** Presence URI through which the OIC devices advertise
 * their device presence.*/
Iotivity.prototype.OC_RSRVD_DEVICE_PRESENCE_URI = "/oic/prs";

/** Sets the default time to live (TTL) for presence.*/
Iotivity.prototype.OC_DEFAULT_PRESENCE_TTL_SECONDS = 60;

/** For multicast Discovery mechanism.*/
Iotivity.prototype.OC_MULTICAST_DISCOVERY_URI = "/oic/res";

/** Separator for multiple query string.*/
Iotivity.prototype.OC_QUERY_SEPARATOR = "&;";

/**
 *  OC_DEFAULT_PRESENCE_TTL_SECONDS sets the default time to
 *  live (TTL) for presence.
 */
Iotivity.prototype.OC_DEFAULT_PRESENCE_TTL_SECONDS = 60;

/**
 *  OC_MAX_PRESENCE_TTL_SECONDS sets the maximum time to live (TTL)
 *  for presence. NOTE: Changing the setting to a longer duration
 *  may lead to unsupported and untested operation.
 *  60 sec/min * 60 min/hr * 24 hr/day
 */
Iotivity.prototype.OC_MAX_PRESENCE_TTL_SECONDS = 60 * 60 * 24;


/**
 *  Presence "Announcement Triggers".
 */

/** To create.*/
Iotivity.prototype.OC_RSRVD_TRIGGER_CREATE = "create";

/** To change.*/
Iotivity.prototype.OC_RSRVD_TRIGGER_CHANGE = "change";

/** To delete.*/
Iotivity.prototype.OC_RSRVD_TRIGGER_DELETE = "delete";

/**
 *  Attributes used to form a proper OIC conforming JSON message.
 */

Iotivity.prototype.OC_RSRVD_OC = "oic";


/** For payload. */

Iotivity.prototype.OC_RSRVD_PAYLOAD = "payload";

/** To represent href */
Iotivity.prototype.OC_RSRVD_HREF = "href";

/** To represent property*/
Iotivity.prototype.OC_RSRVD_PROPERTY = "prop";

/** For representation.*/
Iotivity.prototype.OC_RSRVD_REPRESENTATION = "rep";

/** To represent content type.*/
Iotivity.prototype.OC_RSRVD_CONTENT_TYPE = "ct";

/** To represent resource type.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE = "rt";

/** To represent resource type with presence.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_PRESENCE = "oic.wk.ad";

/** To represent resource type with device.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_DEVICE = "oic.wk.d";

/** To represent resource type with platform.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_PLATFORM = "oic.wk.p";

/** To represent resource type with collection.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_COLLECTION = "oic.wk.col";

/** To represent resource type with RES.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_RES = "oic.wk.res";

/** To represent content type with MQ Broker.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_MQ_BROKER = "oic.wk.ps";

/** To represent content type with MQ Topic.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_MQ_TOPIC = "oic.wk.ps.topic";


/** To represent interface.*/
Iotivity.prototype.OC_RSRVD_INTERFACE = "if";

/** To indicate how long RD should publish this item.*/
Iotivity.prototype.OC_RSRVD_DEVICE_TTL = "lt";

/** To represent time to live.*/
Iotivity.prototype.OC_RSRVD_TTL = "ttl";

/** To represent non*/
Iotivity.prototype.OC_RSRVD_NONCE = "non";

/** To represent trigger type.*/
Iotivity.prototype.OC_RSRVD_TRIGGER = "trg";

/** To represent links.*/
Iotivity.prototype.OC_RSRVD_LINKS = "links";

/** To represent default interface.*/
Iotivity.prototype.OC_RSRVD_INTERFACE_DEFAULT = "oic.if.baseline";

/** To represent read-only interface.*/
Iotivity.prototype.OC_RSRVD_INTERFACE_READ = "oic.if.r";

/** To represent ll interface.*/
Iotivity.prototype.OC_RSRVD_INTERFACE_LL = "oic.if.ll";

/** To represent batch interface.*/
Iotivity.prototype.OC_RSRVD_INTERFACE_BATCH = "oic.if.b";

/** To represent interface group.*/
Iotivity.prototype.OC_RSRVD_INTERFACE_GROUP = "oic.mi.grp";

/** To represent MFG date.*/
Iotivity.prototype.OC_RSRVD_MFG_DATE = "mndt";

/** To represent FW version.*/
Iotivity.prototype.OC_RSRVD_FW_VERSION = "mnfv";

/** To represent host name.*/
Iotivity.prototype.OC_RSRVD_HOST_NAME = "hn";

/** To represent policy.*/
Iotivity.prototype.OC_RSRVD_POLICY = "p";

/** To represent bitmap.*/
Iotivity.prototype.OC_RSRVD_BITMAP = "bm";

/** For security.*/
Iotivity.prototype.OC_RSRVD_SECURE = "sec";

/** Port. */
Iotivity.prototype.OC_RSRVD_HOSTING_PORT = "port";

/** TCP Port. */
Iotivity.prototype.OC_RSRVD_TCP_PORT = "tcp";

/** TLS Port. */
Iotivity.prototype.OC_RSRVD_TLS_PORT = "tls";

/** For Server instance ID.*/
Iotivity.prototype.OC_RSRVD_SERVER_INSTANCE_ID = "sid";

/**
 *  Platform.
 */

/** Platform ID. */
Iotivity.prototype.OC_RSRVD_PLATFORM_ID = "pi";

/** Platform MFG NAME. */
Iotivity.prototype.OC_RSRVD_MFG_NAME = "mnmn";

/** Platform URL. */
Iotivity.prototype.OC_RSRVD_MFG_URL = "mnml";

/** Model Number.*/
Iotivity.prototype.OC_RSRVD_MODEL_NUM = "mnmo";

/** Platform MFG Date.*/
Iotivity.prototype.OC_RSRVD_MFG_DATE = "mndt";

/** Platform versio.n */
Iotivity.prototype.OC_RSRVD_PLATFORM_VERSION = "mnpv";

/** Platform Operating system version. */
Iotivity.prototype.OC_RSRVD_OS_VERSION = "mnos";

/** Platform Hardware version. */
Iotivity.prototype.OC_RSRVD_HARDWARE_VERSION = "mnhw";

/**Platform Firmware version. */
Iotivity.prototype.OC_RSRVD_FIRMWARE_VERSION = "mnfv";

/** Support URL for the platform. */
Iotivity.prototype.OC_RSRVD_SUPPORT_URL = "mnsl";

/** System time for the platform. */
Iotivity.prototype.OC_RSRVD_SYSTEM_TIME = "st";

/** VID for the platform. */
Iotivity.prototype.OC_RSRVD_VID = "vid";
/**
 *  Device.
 */

/** Device ID.*/
Iotivity.prototype.OC_RSRVD_DEVICE_ID = "di";

/** Device Name.*/
Iotivity.prototype.OC_RSRVD_DEVICE_NAME = "n";

/** Device specification version.*/
Iotivity.prototype.OC_RSRVD_SPEC_VERSION = "icv";

/** Device data model.*/
Iotivity.prototype.OC_RSRVD_DATA_MODEL_VERSION = "dmv";

/** Device specification version.*/
Iotivity.prototype.OC_SPEC_VERSION = "core.1.1.0";

/** Device Data Model version.*/
Iotivity.prototype.OC_DATA_MODEL_VERSION = "res.1.1.0,sh.1.1.0";

/** Multicast Prefix.*/
Iotivity.prototype.OC_MULTICAST_PREFIX = "224.0.1.187:5683";

/** Multicast IP address.*/
Iotivity.prototype.OC_MULTICAST_IP = "224.0.1.187";

/** Multicast Port.*/
Iotivity.prototype.OC_MULTICAST_PORT = 5683;

/** Max Device address size. */
Iotivity.prototype.MAX_ADDR_STR_SIZE = 256;

/** Length of MAC address */
Iotivity.prototype.MAC_ADDR_STR_SIZE = 17;

/** Blocks of MAC address */
Iotivity.prototype.MAC_ADDR_BLOCKS = 6;

/** Max identity size. */
Iotivity.prototype.MAX_IDENTITY_SIZE = 37;

/** Universal unique identity size. */
Iotivity.prototype.UUID_IDENTITY_SIZE = 128 / 8;

/** Resource Directory */

/** Resource Directory URI used to Discover RD and Publish resources.*/
Iotivity.prototype.OC_RSRVD_RD_URI = "/oic/rd";

/** To represent resource type with rd.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_RD = "oic.wk.rd";

/** RD Discovery bias factor type. */
Iotivity.prototype.OC_RSRVD_RD_DISCOVERY_SEL = "sel";

/** Resource URI used to discover Proxy */
Iotivity.prototype.OC_RSRVD_PROXY_URI = "/oic/chp";

/** Resource URI used to discover Proxy */
Iotivity.prototype.OC_RSRVD_PROXY_OPTION_ID = 35;

/** Base URI. */
Iotivity.prototype.OC_RSRVD_BASE_URI = "baseURI";

/** Unique value per collection/link. */
Iotivity.prototype.OC_RSRVD_INS = "ins";

/** Allowable resource types in the links. */
Iotivity.prototype.OC_RSRVD_RTS = "rts";

/** Default relationship. */
Iotivity.prototype.OC_RSRVD_DREL = "drel";

/** Defines relationship between links. */
Iotivity.prototype.OC_RSRVD_REL = "rel";

/** Defines title. */
Iotivity.prototype.OC_RSRVD_TITLE = "title";

/** Defines URI. */
Iotivity.prototype.OC_RSRVD_URI = "anchor";

/** Defines media type. */
Iotivity.prototype.OC_RSRVD_MEDIA_TYPE = "type";

/** To represent resource type with Publish RD.*/
Iotivity.prototype.OC_RSRVD_RESOURCE_TYPE_RDPUBLISH = "oic.wk.rdpub";

/** Cloud Account */

/** Account URI.*/
Iotivity.prototype.OC_RSRVD_ACCOUNT_URI = "/oic/account";

/** Account user URI.*/
Iotivity.prototype.OC_RSRVD_ACCOUNT_SEARCH_URI = "/oic/account/search";

/** Account session URI.*/
Iotivity.prototype.OC_RSRVD_ACCOUNT_SESSION_URI = "/oic/account/session";

/** Account token refresh URI.*/
Iotivity.prototype.OC_RSRVD_ACCOUNT_TOKEN_REFRESH_URI =
  "/oic/account/tokenrefresh";

/** ACL group URI.*/
Iotivity.prototype.OC_RSRVD_ACL_GROUP_URI = "/oic/acl/group";

/** ACL invite URI.*/
Iotivity.prototype.OC_RSRVD_ACL_INVITE_URI = "/oic/acl/invite";

/** Defines auth provider. */
Iotivity.prototype.OC_RSRVD_AUTHPROVIDER = "authprovider";

/** Defines auth code. */
Iotivity.prototype.OC_RSRVD_AUTHCODE = "authcode";

/** Defines access token. */
Iotivity.prototype.OC_RSRVD_ACCESS_TOKEN = "accesstoken";

/** Defines login. */
Iotivity.prototype.OC_RSRVD_LOGIN = "login";

/** Defines search. */
Iotivity.prototype.OC_RSRVD_SEARCH = "search";

/** Defines grant type. */
Iotivity.prototype.OC_RSRVD_GRANT_TYPE = "granttype";

/** Defines refresh token. */
Iotivity.prototype.OC_RSRVD_REFRESH_TOKEN = "refreshtoken";

/** Defines user UUID. */
Iotivity.prototype.OC_RSRVD_USER_UUID = "uid";

/** Defines group ID. */
Iotivity.prototype.OC_RSRVD_GROUP_ID = "gid";

/** Defines member of group ID. */
Iotivity.prototype.OC_RSRVD_MEMBER_ID = "mid";

/** Defines invite. */
Iotivity.prototype.OC_RSRVD_INVITE = "invite";

/** Defines accept. */
Iotivity.prototype.OC_RSRVD_ACCEPT = "accept";

/** Defines operation. */
Iotivity.prototype.OC_RSRVD_OPERATION = "op";

/** Defines add. */
Iotivity.prototype.OC_RSRVD_ADD = "add";

/** Defines delete. */
Iotivity.prototype.OC_RSRVD_DELETE = "delete";

/** Defines owner. */
Iotivity.prototype.OC_RSRVD_OWNER = "owner";

/** Defines members. */
Iotivity.prototype.OC_RSRVD_MEMBERS = "members";

/** To represent grant type with refresh token. */
Iotivity.prototype.OC_RSRVD_GRANT_TYPE_REFRESH_TOKEN = "refresh_token";

/** Cloud CRL */
Iotivity.prototype.OC_RSRVD_PROV_CRL_URL = "/oic/credprov/crl";

Iotivity.prototype.OC_RSRVD_LAST_UPDATE = "lu";

Iotivity.prototype.OC_RSRVD_THIS_UPDATE = "tu";

Iotivity.prototype.OC_RSRVD_NEXT_UPDATE = "nu";

Iotivity.prototype.OC_RSRVD_SERIAL_NUMBERS = "rcsn";

Iotivity.prototype.OC_RSRVD_CRL = "crl";

Iotivity.prototype.OC_RSRVD_CRL_ID = "crlid";

/** Cloud ACL */
Iotivity.prototype.OC_RSRVD_GROUP_URL = "/oic/group";

Iotivity.prototype.OC_RSRVD_ACL_GROUP_URL = "/oic/acl/group";

Iotivity.prototype.OC_RSRVD_ACL_INVITE_URL = "/oic/acl/invite";

Iotivity.prototype.OC_RSRVD_ACL_VERIFY_URL = "/oic/acl/verify";

Iotivity.prototype.OC_RSRVD_ACL_ID_URL = "/oic/acl/id";

Iotivity.prototype.OC_RSRVD_MEMBER_ID = "mid";

Iotivity.prototype.OC_RSRVD_GROUP_ID = "gid";

Iotivity.prototype.OC_RSRVD_OWNER_ID = "oid";

Iotivity.prototype.OC_RSRVD_ACL_ID = "aclid";

Iotivity.prototype.OC_RSRVD_ACE_ID = "aceid";

Iotivity.prototype.OC_RSRVD_DEVICE_ID = "di";

Iotivity.prototype.OC_RSRVD_SUBJECT_ID = "sid";

Iotivity.prototype.OC_RSRVD_REQUEST_METHOD = "rm";

Iotivity.prototype.OC_RSRVD_REQUEST_URI = "uri";

Iotivity.prototype.OC_RSRVD_GROUP_MASTER_ID = "gmid";

Iotivity.prototype.OC_RSRVD_GROUP_TYPE = "gtype";

Iotivity.prototype.OC_RSRVD_SUBJECT_TYPE = "stype";

Iotivity.prototype.OC_RSRVD_GROUP_ID_LIST = "gidlist";

Iotivity.prototype.OC_RSRVD_MEMBER_ID_LIST = "midlist";

Iotivity.prototype.OC_RSRVD_DEVICE_ID_LIST = "dilist";

Iotivity.prototype.OC_RSRVD_ACCESS_CONTROL_LIST = "aclist";

Iotivity.prototype.OC_RSRVD_RESOURCES = "resources";

Iotivity.prototype.OC_RSRVD_VALIDITY = "validity";

Iotivity.prototype.OC_RSRVD_PERIOD = "period";

Iotivity.prototype.OC_RSRVD_RECURRENCE = "recurrence";

Iotivity.prototype.OC_RSRVD_INVITE = "invite";

Iotivity.prototype.OC_RSRVD_INVITED = "invited";

Iotivity.prototype.OC_RSRVD_ENCODING = "encoding";

Iotivity.prototype.OC_OIC_SEC = "oic.sec";

Iotivity.prototype.OC_RSRVD_BASE64 = "base64";

Iotivity.prototype.OC_RSRVD_DER = "der";

Iotivity.prototype.OC_RSRVD_PEM = "pem";

Iotivity.prototype.OC_RSRVD_RAW = "raw";

Iotivity.prototype.OC_RSRVD_UNKNOWN = "unknown";

Iotivity.prototype.OC_RSRVD_DATA = "data";

Iotivity.prototype.OC_RSRVD_RESOURCE_OWNER_UUID = "rowneruuid";

Iotivity.prototype.OC_RSRVD_SUBJECT_UUID = "subjectuuid";

Iotivity.prototype.OC_RSRVD_PERMISSION_MASK = "permission";

Iotivity.prototype.OC_RSRVD_GROUP_PERMISSION = "gp";

Iotivity.prototype.OC_RSRVD_GROUP_ACL = "gacl";

/** Certificete Sign Request */
Iotivity.prototype.OC_RSRVD_PROV_CERT_URI = "/oic/credprov/cert";

Iotivity.prototype.OC_RSRVD_CSR = "csr";

Iotivity.prototype.OC_RSRVD_CERT = "cert";

Iotivity.prototype.OC_RSRVD_CACERT = "certchain";

Iotivity.prototype.OC_RSRVD_TOKEN_TYPE = "tokentype";

Iotivity.prototype.OC_RSRVD_EXPIRES_IN = "expiresin";

Iotivity.prototype.OC_RSRVD_REDIRECT_URI = "redirecturi";

Iotivity.prototype.OC_RSRVD_CERTIFICATE = "certificate";


/**
 * These enums (OCTransportAdapter and OCTransportFlags) must
 * be kept synchronized with OCConnectivityType (below) as well as
 * CATransportAdapter and CATransportFlags (in CACommon.h).
 */


/**
 *  Enum layout assumes some targets have 16-bit integer (e.g., Arduino).
 */
Iotivity.prototype.OCTransportFlags = {
  /** default flag is 0*/
  OC_DEFAULT_FLAGS: 0,

  /** Insecure transport is the default (subject to change).*/
  /** secure the transport path*/
  OC_FLAG_SECURE: (1 << 4),

  /** IPv4 & IPv6 auto-selection is the default.*/
  /** IP & TCP adapter only.*/
  OC_IP_USE_V6: (1 << 5),

  /** IP & TCP adapter only.*/
  OC_IP_USE_V4: (1 << 6),

  /** Multicast only.*/
  OC_MULTICAST: (1 << 7),

  /** Link-Local multicast is the default multicast scope for IPv6.
   *  These are placed here to correspond to the IPv6 multicast
   * address bits.*/

  /** IPv6 Interface-Local scope (loopback).*/
  OC_SCOPE_INTERFACE: 0x1,

  /** IPv6 Link-Local scope (default).*/
  OC_SCOPE_LINK: 0x2,

  /** IPv6 Realm-Local scope. */
  OC_SCOPE_REALM: 0x3,

  /** IPv6 Admin-Local scope. */
  OC_SCOPE_ADMIN: 0x4,

  /** IPv6 Site-Local scope. */
  OC_SCOPE_SITE: 0x5,

  /** IPv6 Organization-Local scope. */
  OC_SCOPE_ORG: 0x8,

  /**IPv6 Global scope. */
  OC_SCOPE_GLOBAL: 0xE


};

/** Bit mask for scope.*/
Iotivity.prototype.OC_MASK_SCOPE = 0x000F;

/** Bit mask for Mods.*/
Iotivity.prototype.OC_MASK_MODS = 0x0FF0;
Iotivity.prototype.OC_MASK_FAMS =
  Iotivity.prototype.OCTransportFlags.OC_IP_USE_V6 |
  Iotivity.prototype.OCTransportFlags.OC_IP_USE_V4;

/**
 * This enum type includes elements of both ::OCTransportAdapter
 * and ::OCTransportFlags. It is defined conditionally because the smaller
 * definition limits expandability on 32/64 bit
 * integer machines, and the larger definition won't fit into
 * an enum on 16-bit integer machines like Arduino.
 *
 * This structure must directly correspond to ::OCTransportAdapter
 * and ::OCTransportFlags.
 */
Iotivity.prototype.OCConnectivityType = {
  /** use when defaults are ok. */
  CT_DEFAULT: 0,

  /** IPv4 and IPv6, including 6LoWPAN.*/
  CT_ADAPTER_IP: (1 << 16),

  /** GATT over Bluetooth LE.*/
  CT_ADAPTER_GATT_BTLE: (1 << 17),

  /** RFCOMM over Bluetooth EDR.*/
  CT_ADAPTER_RFCOMM_BTEDR: (1 << 18),

  /** Remote Access over XMPP.*/
  CT_ADAPTER_REMOTE_ACCESS: (1 << 19),

  /** CoAP over TCP.*/
  CT_ADAPTER_TCP: (1 << 20),

  /** NFC Transport.*/
  CT_ADAPTER_NFC: (1 << 21),

  /** Insecure transport is the default (subject to change).*/

  /** secure the transport path.*/
  CT_FLAG_SECURE: (1 << 4),

  /** IPv4 & IPv6 autoselection is the default.*/

  /** IP adapter only.*/
  CT_IP_USE_V6: (1 << 5),

  /** IP adapter only.*/
  CT_IP_USE_V4: (1 << 6),

  /** Link-Local multicast is the default multicast scope for IPv6.
   * These are placed here to correspond to the IPv6 address bits.*/

  /** IPv6 Interface-Local scope(loopback).*/
  CT_SCOPE_INTERFACE: 0x1,

  /** IPv6 Link-Local scope (default).*/
  CT_SCOPE_LINK: 0x2,

  /** IPv6 Realm-Local scope.*/
  CT_SCOPE_REALM: 0x3,

  /** IPv6 Admin-Local scope.*/
  CT_SCOPE_ADMIN: 0x4,

  /** IPv6 Site-Local scope.*/
  CT_SCOPE_SITE: 0x5,

  /** IPv6 Organization-Local scope.*/
  CT_SCOPE_ORG: 0x8,

  /** IPv6 Global scope.*/
  CT_SCOPE_GLOBAL: 0xE

};

/** bit shift required for connectivity adapter.*/
Iotivity.prototype.CT_ADAPTER_SHIFT = 16;

/** Mask Flag.*/
Iotivity.prototype.CT_MASK_FLAGS = 0xFFFF;

/** Mask Adapter.*/
Iotivity.prototype.CT_MASK_ADAPTER = 0xFFFF0000;

/**
 *  OCDoResource methods to dispatch the request
 */
Iotivity.prototype.OCMethod = {
  OC_REST_NOMETHOD: 0,

  /** Read.*/
  OC_REST_GET: (1 << 0),

  /** Write.*/
  OC_REST_PUT: (1 << 1),

  /** Update.*/
  OC_REST_POST: (1 << 2),

  /** Delete.*/
  OC_REST_DELETE: (1 << 3),

  /** Register observe request for most up date notifications ONLY.*/
  OC_REST_OBSERVE: (1 << 4),

  /** Register observe request for all notifications,
   * including stale notifications.*/
  OC_REST_OBSERVE_ALL: (1 << 5),

  /** Subscribe for all presence notifications of a particular resource.*/
  OC_REST_PRESENCE: (1 << 7),

  /** Allows OCDoResource caller to do discovery.*/
  OC_REST_DISCOVER: (1 << 8)
};

/**
 *  Formats for payload encoding.
 */
Iotivity.prototype.OCPayloadFormat = {
  OC_FORMAT_CBOR: 0,
  OC_FORMAT_JSON: 1,
  OC_FORMAT_UNDEFINED: 2,
  OC_FORMAT_UNSUPPORTED: 3
};

/**
 * Host Mode of Operation.
 */
Iotivity.prototype.OCMode = {
  OC_CLIENT: 0,
  OC_SERVER: 1,
  OC_CLIENT_SERVER: 2,

  /**< Client server mode along with routing capabilities.*/
  OC_GATEWAY: 3

};

/**
 * Quality of Service attempts to abstract the
 * guarantees provided by the underlying transport
 * protocol. The precise definitions of each
 * quality of service level depend on the
 * implementation. In descriptions below are for
 * the current implementation and may changed
 * over time.
 */
Iotivity.prototype.OCQualityOfService = {
  /** Packet delivery is best effort.*/
  OC_LOW_QOS: 0,

  /** Packet delivery is best effort.*/
  OC_MEDIUM_QOS: 1,

  /** Acknowledgments are used to confirm delivery.*/
  OC_HIGH_QOS: 2,

  /** No Quality is defined, let the stack decide.*/
  OC_NA_QOS: 3

};

/**
 * Resource Properties.
 * The value of a policy property is defined as bitmap.
 * The LSB represents OC_DISCOVERABLE and
 * Second LSB bit represents OC_OBSERVABLE and so on.
 * Not including the policy property is equivalent to zero.
 *
 */
Iotivity.prototype.OCResourceProperty = {
  /**
   * When none of the bits are set, the resource is non-discoverable &
   *  non-observable by the client.
   */
  OC_RES_PROP_NONE: (0),

  /**
   * When this bit is set, the resource
   *  is allowed to be discovered by clients.
   */
  OC_DISCOVERABLE: (1 << 0),

  /**
   * When this bit is set, the resource
   *  is allowed to be observed by clients.
   */
  OC_OBSERVABLE: (1 << 1),

  /**
   * When this bit is set, the resource
   * is initialized, otherwise the resource
   * is 'inactive'. 'inactive' signifies that the
   * resource has been marked for
   * deletion or is already deleted.
   */
  OC_ACTIVE: (1 << 2),

  /**
   * When this bit is set, the resource has been marked as 'slow'.
   * 'slow' signifies that responses from this resource can expect delays in
   *  processing its requests from clients.
   */
  OC_SLOW: (1 << 3),

  OC_SECURE: (0),

  /**
   * When this bit is set, the resource is allowed to be discovered only
   * if discovery request contains an explicit querystring.
   * Ex: GET /oic/res?rt=oic.sec.acl
   */
  OC_EXPLICIT_DISCOVERABLE: (1 << 5),

  /** When this bit is set, the resource is allowed to be published */
  OC_MQ_PUBLISHER: (1 << 6),

  /** When this bit is set, the resource is allowed
   *  to be notified as MQ broker.*/
  OC_MQ_BROKER: (1 << 7)


};

/**
 * Transport Protocol IDs.
 */
Iotivity.prototype.OCTransportProtocolID = {
  /** For invalid ID.*/
  OC_INVALID_ID: (1 << 0),

  /* For coap ID.*/
  OC_COAP_ID: (1 << 1)

};

/**
 * Declares Stack Results & Errors.
 */
Iotivity.prototype.OCStackResult = {
  /** Success status code - START HERE.*/
  OC_STACK_OK: 0,
  OC_STACK_RESOURCE_CREATED: 1,
  OC_STACK_RESOURCE_DELETED: 2,
  OC_STACK_CONTINUE: 3,
  OC_STACK_RESOURCE_CHANGED: 4,
  /** Success status code - END HERE.*/

  /** Error status code - START HERE.*/
  OC_STACK_INVALID_URI: 20,
  OC_STACK_INVALID_QUERY: 21,
  OC_STACK_INVALID_IP: 22,
  OC_STACK_INVALID_PORT: 23,
  OC_STACK_INVALID_CALLBACK: 24,
  OC_STACK_INVALID_METHOD: 25,

  /** Invalid parameter.*/
  OC_STACK_INVALID_PARAM: 26,
  OC_STACK_INVALID_OBSERVE_PARAM: 27,
  OC_STACK_NO_MEMORY: 28,
  OC_STACK_COMM_ERROR: 29,
  OC_STACK_TIMEOUT: 30,
  OC_STACK_ADAPTER_NOT_ENABLED: 31,
  OC_STACK_NOTIMPL: 32,

  /** Resource not found.*/
  OC_STACK_NO_RESOURCE: 33,

  /** e.g: not supported method or interface.*/
  OC_STACK_RESOURCE_ERROR: 34,
  OC_STACK_SLOW_RESOURCE: 35,
  OC_STACK_DUPLICATE_REQUEST: 36,

  /** Resource has no registered observers.*/
  OC_STACK_NO_OBSERVERS: 37,
  OC_STACK_OBSERVER_NOT_FOUND: 38,
  OC_STACK_VIRTUAL_DO_NOT_HANDLE: 39,
  OC_STACK_INVALID_OPTION: 40,

  /** The remote reply contained malformed data.*/
  OC_STACK_MALFORMED_RESPONSE: 41,
  OC_STACK_PERSISTENT_BUFFER_REQUIRED: 42,
  OC_STACK_INVALID_REQUEST_HANDLE: 43,
  OC_STACK_INVALID_DEVICE_INFO: 44,
  OC_STACK_INVALID_JSON: 45,

  /** Request is not authorized by Resource Server. */
  OC_STACK_UNAUTHORIZED_REQ: 46,
  OC_STACK_TOO_LARGE_REQ: 47,

  /** Error code from PDM */
  OC_STACK_PDM_IS_NOT_INITIALIZED: 48,
  OC_STACK_DUPLICATE_UUID: 49,
  OC_STACK_INCONSISTENT_DB: 50,

  /**
   * Error code from OTM
   * This error is pushed from DTLS interface when handshake failure happens
   */
  OC_STACK_AUTHENTICATION_FAILURE: 51,
  OC_STACK_NOT_ALLOWED_OXM: 52,

  /** Insert all new error codes here!.*/
  OC_STACK_PRESENCE_STOPPED: 128,
  OC_STACK_PRESENCE_TIMEOUT: 129,
  OC_STACK_PRESENCE_DO_NOT_HANDLE: 130,

  /** ERROR code from server */ /* WITH PRESENSE!!!! else 53 and 54!*/
  OC_STACK_FORBIDDEN_REQ: WITH_PRESENCE ? 131 : 53,          /** 403*/
  OC_STACK_INTERNAL_SERVER_ERROR: WITH_PRESENCE ? 132 : 54,  /** 500*/

  /** ERROR in stack.*/
  OC_STACK_ERROR: 255,
  /** Error status code - END HERE.*/

};

Iotivity.prototype.MAX_SEQUENCE_NUMBER = 0xFFFFFF;

Iotivity.prototype.OCObserveAction = {
  OC_OBSERVE_REGISTER: 0,
  OC_OBSERVE_DEREGISTER: 1,
  OC_OBSERVE_NO_OPTION: 2
};

Iotivity.prototype.OCEntityHandlerResult = {
  OC_EH_OK: 0,
  OC_EH_ERROR: 1,
  OC_EH_SLOW: 2,
  OC_EH_RESOURCE_CREATED: 201,
  OC_EH_RESOURCE_DELETED: 202,
  OC_EH_VALID: 203,
  OC_EH_CHANGED: 204,
  OC_EH_CONTENT: 205,
  OC_EH_BAD_REQ: 400,
  OC_EH_UNAUTHORIZED_REQ: 401,
  OC_EH_BAD_OPT: 402,
  OC_EH_FORBIDDEN: 403,
  OC_EH_RESOURCE_NOT_FOUND: 404,
  OC_EH_METHOD_NOT_ALLOWED: 405,
  OC_EH_NOT_ACCEPTABLE: 406,
  OC_EH_TOO_LARGE: 413,
  OC_EH_UNSUPPORTED_MEDIA_TYPE: 415,
  OC_EH_INTERNAL_SERVER_ERROR: 500,
  OC_EH_BAD_GATEWAY: 502,
  OC_EH_SERVICE_UNAVAILABLE: 503,
  OC_EH_RETRANSMIT_TIMEOUT: 504
};

Iotivity.prototype.OCPayloadType = {
  PAYLOAD_TYPE_INVALID: 0,
  PAYLOAD_TYPE_DISCOVERY: 1,
  PAYLOAD_TYPE_DEVICE: 2,
  PAYLOAD_TYPE_PLATFORM: 3,
  PAYLOAD_TYPE_REPRESENTATION: 4,
  PAYLOAD_TYPE_SECURITY: 5,
  PAYLOAD_TYPE_PRESENCE: 6
};

Iotivity.prototype.OCRepPayloadPropType = {
  OCREP_PROP_NULL: 0,
  OCREP_PROP_INT: 1,
  OCREP_PROP_DOUBLE: 2,
  OCREP_PROP_BOOL: 3,
  OCREP_PROP_STRING: 4,
  OCREP_PROP_BYTE_STRING: 5,
  OCREP_PROP_OBJECT: 6,
  OCREP_PROP_ARRAY: 7
};

Iotivity.prototype.OCEntityHandlerFlag = {
  OC_REQUEST_FLAG: (1 << 1),
  OC_OBSERVE_FLAG: (1 << 2)
};

Iotivity.prototype.OCStackApplicationResult = {
  OC_STACK_DELETE_TRANSACTION: 0,
  OC_STACK_KEEP_TRANSACTION: 1
};



module.exports = new Iotivity();
module.exports.Iotivity = Iotivity;
