import statistics
import sys
import re
import datetime

def difftime(x, y):
    try:
        v = x.nanotime - y.nanotime
        if v <= 0:
            return None
        return x.nanotime - y.nanotime
    except Exception as e:
        return None

class log:
    def __init__(self, line):
        self.timestamp = datetime.datetime.strptime(line[0], "[%Y-%m-%dT%H:%M:%S.%f]")
        self.info = line[1]

        if not line[2].startswith("[#tid"):
            raise ValueError("tid wrong")
        self.tid = line[2]

        self.place = line[3]

        if re.match("^[<][-+]?[0-9]+[>]$", line[4]):
            self.nanotime = int(line[4][1:-1])
            self.other = " ".join(line[5:])
        else:
            self.nanotime = None
            self.other = " ".join(line[4:])

class request:
    def __init__(self):
        self.post = None                # POST /api/v1/namespaces/_/actions/direct-dev-hello blocking=true
        self.controllerRoute = None     # [Controller] <86646692935> [EXTRA] Controller.route() called *
        self.ApiRoute = None            # [RestAPIVersion] <87940590975> RestAPIs.routes() *
        self.Auth = None                # [BasicAuthenticationDirective] <87941520671> [EXTRA] authenticate() called *
        self.AuthStart = None           # [BasicAuthenticationDirective] <87942071701> [EXTRA] BasicAuthenticationAuthKey() called *
        self.AuthDone = None            # [BasicAuthenticationDirective] <87942218697> [EXTRA] BasicAuthenticationAuthKey() done *
        self.IdentityCacheLookup = None # [Identity] <87357279571> [EXTRA] cacheLookup() called
        self.IdentityCacheDone = None   # [Identity] <87358400350> [GET] serving from cache: CacheKey(...

        self.innerRoutes = None         # [EXTRA] innerRoutes() called
        self.checkAuthentication = None # [BasicAuthenticationDirective] <87358610139> authentication valid
        self.checkPrivilege = None      # [LocalEntitlementProvider] <87359610377> checking user
        self.RateThrottler = None       # [RateThrottler] <87359702692> namespace
        self.ActivationThrottler = None # [ActivationThrottler] <87359989975> namespace
        self.LocalEntitlementProviderAuthorized = None # [LocalEntitlementProvider] <87360302657> authorized

        self.activate = None                           # [ActionsApi] <87360423241> [EXTRA] activate() called
        self.WhiskActionMetaDataCacheLookup = None     # [WhiskActionMetaData] <87361219117> [EXTRA] cacheLookup() called
        self.WhiskActionMetaDataCacheRead = None       # [WhiskActionMetaData] <87362087422> cached read
        self.WhiskActionMetaDataCacheDone = None       # [WhiskActionMetaData] <87362341890> [GET] serving from cache:

        self.ActionsApiGetEntitySuccess = None # [ActionsApi] <87362517785> [GET] entity success
        self.ActionsApiDoInvoke = None         # [ActionsApi] <87362687364> [EXTRA] doInvoke() called

        self.ShardingContainerPoolBalancerScheduledActivation = None # [ShardingContainerPoolBalancer] <87363674759> scheduled activation
        self.ShardingContainerPoolBalancerPostingTopic = None       # [ShardingContainerPoolBalancer] <87363954549> posting topic 'invoker1'

        self.KafkaSent = None      # [KafkaProducerConnector] <87368931318> sent message
        self.StartWaiting = None   # [ActionsApi] <87369426345> action activation will block for result upto

        self.KafkaResultAck   = None # [ShardingContainerPoolBalancer] <87448716364> received result ack for
        self.KafkaCompleteAck = None # [ShardingContainerPoolBalancer] <87451778075> received completion ack

        self.doinvokeOncomplete = None # [ActionsApi] <88126475066> [EXTRA] doInvoke().onComplete() called (block) *
        self.invokeSingleAction = None # [ActionsApi] <88151539353> [EXTRA] invokeSingleAction() called *
        self.invokeSingleActionPost = None # [ActionsApi] <88152408377> [EXTRA] invokeSimpleAction().postmessage called *
        self.invokeSingleActionPostDone = None # [ActionsApi] <88154275374> [EXTRA] invokeSimpleAction().postmessage finished *
        self.invokeSingleActionDone = None # [ActionsApi] <88268638608> [EXTRA] invokeSimpleAction().andThenSuccess finished *
        self.finish = None # [BasicHttpService] [marker:http_post.200_counter:103:103]

        # invoker
        self.InvokerReactive          = None # [InvokerReactive] <46222178731>  [marker:invoker_activation_start:257]
        self.InvokerHandleActivationMessage = None # [InvokerReactive] <46222439910> guest/direct-dev-hello guest 182930a351284072a930a351286072b5
        self.InvokerCacheLookup       = None # [WhiskAction] <48903193779> [EXTRA] cacheLookup() called
        self.InvokerCacheServed       = None # [WhiskAction] <48905675157> [GET] serving from cache: CacheKey(guest
        self.InvokerContainerStarting = None # [ContainerPool] <48906513579> containerStart containerState
        self.InvokerContainerResume   = None # [RuncClient] <48933338841> running /usr/bin/docker-runc resume
        self.InvokerContainerStarted  = None # [RuncClient] <48945106601>  [marker:invoker_runc.resume_finish

        self.InvokerSendArgument      = None # [DockerContainer] <48954410730> sending arguments to
        self.InvokerGetResponseResult = None # [DockerContainer] <48966055632> running result: ok
        self.InvokerCollectLogStart   = None # [ContainerProxy] <48966965498>  [marker:invoker_collectLogs_start
        self.InvokerCollectLogEnd     = None # [ContainerProxy] <48966965498>  [marker:invoker_collectLogs_finish

        self.InvokerRecordingActivation    = None # [ArtifactActivationStore] <48972096394> recording activation
        self.InvokerSaveDB                 = None # [CouchDbRestStore] <48972865436> [PUT] 'whisk_local_activations'
        self.InvokerSaveDBdone             = None # [RepointableActorRef] <48973714790> activation received processed msg,
        self.InvokerSendKafkaResultStart   = None # [KafkaProducerConnector] <49053250504> sent message: completed0
        self.InvokerSendKafkaResultEnd     = None # [MessagingActiveAck] <48973971375> posted result of activation
        self.InvokerSendKafkaCompleteStart = None # [KafkaProducerConnector] <49122460798> sent message: completed0[0][13]
        self.InvokerSendKafkaCompleteEnd   = None # [MessagingActiveAck] <48977197169> posted completion of activation


    def diffarray_controller(self):
        return [
            ("BasicHttpService", 1000000000 * (self.finish.timestamp.timestamp() - self.post.timestamp.timestamp())),
            ("controllerRouteOverhead", 1000000000 * (self.controllerRoute.timestamp.timestamp() - self.post.timestamp.timestamp())),
            ("controllerRoute", 1000000000 * (self.finish.timestamp.timestamp() - self.controllerRoute.timestamp.timestamp())),

            ("apiV1.routes()", difftime(self.innerRoutes, self.ApiRoute)),
            ("authOverhead", difftime(self.Auth, self.ApiRoute)),
            ("auth()", difftime(self.IdentityCacheDone, self.Auth)),
            ("authorizeAndDispatch()", difftime(self.AuthStart, self.Auth)),
            ("BasicAuthenticationDirective", difftime(self.AuthDone, self.AuthStart)),
            ("AuthDone->IdentityCacheLookup", difftime(self.IdentityCacheLookup, self.AuthDone)),
            ("IdentityCache", difftime(self.IdentityCacheDone, self.IdentityCacheLookup)),
            ("checkAuthentication", difftime(self.innerRoutes, self.IdentityCacheDone)),
            ("innerRoutes()", 1000000000 * (self.finish.timestamp.timestamp() - self.innerRoutes.timestamp.timestamp())),

            ("checkPrivilege", difftime(self.checkPrivilege, self.innerRoutes)),
            ("checkRateThrottler", difftime(self.RateThrottler, self.checkPrivilege)),
            ("checkActivationThrottler", difftime(self.LocalEntitlementProviderAuthorized, self.RateThrottler)),

            ("Controller.route()", difftime(self.activate, self.LocalEntitlementProviderAuthorized)),
            ("activate()", difftime(self.doinvokeOncomplete, self.activate)),
            ("cachelookup", difftime(self.WhiskActionMetaDataCacheDone, self.doinvokeOncomplete)),

            ("PreparePreload", difftime(self.ActionsApiDoInvoke, self.WhiskActionMetaDataCacheDone)),
            ("DoInvoke", difftime(self.doinvokeOncomplete, self.ActionsApiDoInvoke)),
            ("invokeSingleAction->doinvokeOncomplete", difftime(self.invokeSingleAction, self.doinvokeOncomplete)),
            ("InvokeSingleAction", difftime(self.invokeSingleActionDone, self.invokeSingleAction)),
            ("postmessage()", difftime(self.invokeSingleActionPostDone, self.invokeSingleActionPost)),
        ]

    def add_controller(self, log):
        if log.place == "POST":
            self.post = log
        elif log.place == "[WebActionsApi]" and log.other.startswith("[EXTRA] routes():1 called"):
#            print(log.nanotime)
            self.WebActionsApiRoute = log  # [WebActionsApi] <87356342992> [EXTRA] routes():1 called
        elif log.place == "[Identity]" and log.other.startswith("[EXTRA] cacheLookup() called"):
            self.IdentityCacheLookup = log # [Identity] <87357279571> [EXTRA] cacheLookup() called
        elif log.place == "[Identity]" and log.other.startswith("cached read"):
            self.IdentityCacheRead = log   # [Identity] <87358197269> cached read
        elif log.place == "[Identity]" and log.other.startswith("[GET] serving from cache: CacheKey"):
            self.IdentityCacheDone = log   # [Identity] <87358400350> [GET] serving from cache: CacheKey(...

        elif log.place == "[Controller]" and log.other.startswith("[EXTRA] Controller.route() called"):
            self.controllerRoute = log     # [Controller] <86646692935> [EXTRA] Controller.route() called *

        elif log.place == "[ActionsApi]" and log.other.startswith("[EXTRA] innerRoutes() called"):
            self.innerRoutes = log         # [ActionsApi] <87011297704> [EXTRA] innerRoutes() called

        elif log.place == "[RestAPIVersion]" and log.other.startswith("RestAPIs.routes()"):
            self.ApiRoute = log            # [RestAPIVersion] <87940590975> RestAPIs.routes() *

        elif log.place == "[BasicAuthenticationDirective]" and log.other.startswith("[EXTRA] authenticate() called"):
            self.Auth = log                # [BasicAuthenticationDirective] <87941520671> [EXTRA] authenticate() called *
        elif log.place == "[BasicAuthenticationDirective]" and log.other.startswith("[EXTRA] BasicAuthenticationAuthKey() called"):
            self.AuthStart = log           # [BasicAuthenticationDirective] <87942071701> [EXTRA] BasicAuthenticationAuthKey() called *
        elif log.place == "[BasicAuthenticationDirective]" and log.other.startswith("[EXTRA] BasicAuthenticationAuthKey() done"):
            self.AuthDone = log            # [BasicAuthenticationDirective] <87942218697> [EXTRA] BasicAuthenticationAuthKey() done *

        elif log.place == "[BasicAuthenticationDirective]" and log.other.startswith("authentication valid"):
            self.checkAuthentication = log # [BasicAuthenticationDirective] <87358610139> authentication valid

        elif log.place == "[LocalEntitlementProvider]" and log.other.startswith("checking user"):
            self.checkPrivilege = log      # [LocalEntitlementProvider] <87359610377> checking user
        elif log.place == "[RateThrottler]" and log.other.startswith("namespace"):
            self.RateThrottler = log       # [RateThrottler] <87359702692> namespace
        elif log.place == "[ActivationThrottler]" and log.other.startswith("namespace"):
            self.ActivationThrottler = log # [ActivationThrottler] <87359989975> namespace
        elif log.place == "[LocalEntitlementProvider]" and log.other.startswith("authorized"):
            self.LocalEntitlementProviderAuthorized = log # [LocalEntitlementProvider] <87360302657> authorized

        elif log.place == "[ActionsApi]" and log.other.startswith("[EXTRA] activate() called"):
            self.activate = log                           # [ActionsApi] <87360423241> [EXTRA] activate() called
        elif log.place == "[WhiskActionMetaData]" and log.other.startswith("[EXTRA] cacheLookup() called"):
            self.WhiskActionMetaDataCacheLookup = log     # [WhiskActionMetaData] <87361219117> [EXTRA] cacheLookup() called
        elif log.place == "[WhiskActionMetaData]" and log.other.startswith("cached read"):
            self.WhiskActionMetaDataCacheRead = log       # [WhiskActionMetaData] <87362087422> cached read
        elif log.place == "[WhiskActionMetaData]" and log.other.startswith("[GET] serving from cache:"):
            self.WhiskActionMetaDataCacheDone = log       # [WhiskActionMetaData] <87362341890> [GET] serving from cache:

        elif log.place == "[ActionsApi]" and log.other.startswith("[GET] entity success"):
            self.ActionsApiGetEntitySuccess = log # [ActionsApi] <87362517785> [GET] entity success
        elif log.place == "[ActionsApi]" and log.other.startswith("[EXTRA] doInvoke() called"):
            self.ActionsApiDoInvoke = log         # [ActionsApi] <87362687364> [EXTRA] doInvoke() called

        elif log.place == "[ShardingContainerPoolBalancer]" and log.other.startswith("scheduled activation"):
            self.ShardingContainerPoolBalancerScheduledActivation = log # [ShardingContainerPoolBalancer] <87363674759> scheduled activation
        elif log.place == "[ShardingContainerPoolBalancer]" and log.other.startswith("posting topic 'invoker1'"):
            self.ShardingContainerPoolBalancerPostingTopic = log       # [ShardingContainerPoolBalancer] <87363954549> posting topic 'invoker1'

        elif log.place == "[KafkaProducerConnector]" and log.other.startswith("sent message"):
            self.KafkaSent = log      # [KafkaProducerConnector] <87368931318> sent message
        elif log.place == "[ActionsApi]" and log.other.startswith("action activation will block for result"):
            self.StartWaiting = log   # [ActionsApi] <87369426345> action activation will block for result upto

        elif log.place == "[ShardingContainerPoolBalancer]" and log.other.startswith("received result ack for"):
            self.KafkaResultAck   = log # [ShardingContainerPoolBalancer] <87448716364> received result ack for
        elif log.place == "[ShardingContainerPoolBalancer]" and log.other.startswith("received completion ack"):
            self.KafkaCompleteAck = log # [ShardingContainerPoolBalancer] <87451778075> received completion ack

        elif log.place == "[ActionsApi]" and log.other.startswith("[EXTRA] doInvoke().onComplete() called"):
            self.doinvokeOncomplete = log # [ActionsApi] <88126475066> [EXTRA] doInvoke().onComplete() called (block) *

        elif log.place == "[ActionsApi]" and log.other.startswith("[EXTRA] invokeSingleAction() called"):
            self.invokeSingleAction = log # [ActionsApi] <88151539353> [EXTRA] invokeSingleAction() called *

        elif log.place == "[ActionsApi]" and log.other.startswith("[EXTRA] invokeSimpleAction().postmessage called"):
            self.invokeSingleActionPost = log # [ActionsApi] <88152408377> [EXTRA] invokeSimpleAction().postmessage called *

        elif log.place == "[ActionsApi]" and log.other.startswith("[EXTRA] invokeSimpleAction().postmessage finished"):
            self.invokeSingleActionPostDone = log # [ActionsApi] <88154275374> [EXTRA] invokeSimpleAction().postmessage finished *

        elif log.place == "[ActionsApi]" and log.other.startswith("[EXTRA] invokeSimpleAction().andThenSuccess"):
            self.invokeSingleActionDone = log # [ActionsApi] <88268638608> [EXTRA] invokeSimpleAction().andThenSuccess finished *

        elif log.place == "[BasicHttpService]" and log.other.startswith("[marker:http_post"):
            self.finish = log # [BasicHttpService] [marker:http_post.200_counter:103:103]

    def add_invoker(self, log):
        if log.place == "[InvokerReactive]" and log.other.startswith("[marker:invoker_activation_start"):
            self.InvokerReactive = log # [InvokerReactive] <46222178731>  [marker:invoker_activation_start

        elif log.place == "[InvokerReactive]" and log.other.startswith("guest"):
            self.InvokerHandleActivationMessage = log # [InvokerReactive] <46222439910> guest/direct-dev-hello guest

        elif log.place == "[WhiskAction]" and log.other.startswith("[EXTRA] cacheLookup() called"):
            self.InvokerCacheLookup       = log # [WhiskAction] <48903193779> [EXTRA] cacheLookup() called

        elif log.place == "[WhiskAction]" and log.other.startswith("[GET] serving from cache: CacheKey("):
            self.InvokerCacheServed       = log # [WhiskAction] <48905675157> [GET] serving from cache: CacheKey(guest

        elif log.place == "[ContainerPool]" and log.other.startswith("containerStart containerState"):
            self.InvokerContainerStarting = log # [ContainerPool] <48906513579> containerStart containerState

        elif log.place == "[RuncClient]" and log.other.startswith("running /usr/bin/docker-runc resume"):
            self.InvokerContainerResume   = log # [RuncClient] <48933338841> running /usr/bin/docker-runc resume

        elif log.place == "[RuncClient]" and log.other.startswith("[marker:invoker_runc.resume_finish"):
            self.InvokerContainerStarted  = log # [RuncClient] <48945106601>  [marker:invoker_runc.resume_finish

        elif log.place == "[DockerContainer]" and log.other.startswith("sending arguments to"):
            self.InvokerSendArgument      = log # [DockerContainer] <48954410730> sending arguments to

        elif log.place == "[DockerContainer]" and log.other.startswith("running result: ok"):
            self.InvokerGetResponseResult = log # [DockerContainer] <48966055632> running result: ok

        elif log.place == "[ContainerProxy]" and log.other.startswith("[marker:invoker_collectLogs_start"):
            self.InvokerCollectLogStart   = log # [ContainerProxy] <48966965498>  [marker:invoker_collectLogs_start

        elif log.place == "[ContainerProxy]" and log.other.startswith("[marker:invoker_collectLogs_finish"):
            self.InvokerCollectLogEnd     = log # [ContainerProxy] <48966965498>  [marker:invoker_collectLogs_finish

        elif log.place == "[ArtifactActivationStore]" and log.other.startswith("recording activation"):
            self.InvokerRecordingActivation    = log # [ArtifactActivationStore] <48972096394> recording activation

        elif log.place == "[CouchDbRestStore]" and log.other.startswith("[PUT] 'whisk_local_activations'"):
            self.InvokerSaveDB                 = log # [CouchDbRestStore] <48972865436> [PUT] 'whisk_local_activations'

#        elif log.place == "[RepointableActorRef]" and "activation received processed msg" in log.other:
#            self.InvokerSaveDBdone             = log # [RepointableActorRef] <48973714790> activation received processed msg,

        elif log.place == "[KafkaProducerConnector]" and log.other.startswith("sent message: completed0") and self.InvokerSendKafkaResultStart == None:
            self.InvokerSendKafkaResultStart   = log # [KafkaProducerConnector] <49053250504> sent message: completed0

        elif log.place == "[MessagingActiveAck]" and log.other.startswith("posted result of activation"):
            self.InvokerSendKafkaResultEnd     = log # [MessagingActiveAck] <48973971375> posted result of activation

        elif log.place == "[KafkaProducerConnector]" and log.other.startswith("sent message: completed0"):
            self.InvokerSendKafkaCompleteStart = log # [KafkaProducerConnector] <49122460798> sent message: completed0[0][13]

        elif log.place == "[MessagingActiveAck]" and log.other.startswith("posted completion of activation"):
            self.InvokerSendKafkaCompleteEnd   = log # [MessagingActiveAck] <48977197169> posted completion of activation

    def diffarray_invoker(self):
        return [
            ("totalInvokerTime", difftime(self.InvokerSendKafkaCompleteEnd, self.InvokerReactive)),
            ("invokerMsgHandler", difftime(self.InvokerContainerStarting, self.InvokerHandleActivationMessage)),
            ("invokerMsgCacheLookup", difftime(self.InvokerCacheServed, self.InvokerCacheLookup)),
            ("containerStartTime", difftime(self.InvokerContainerStarted, self.InvokerContainerStarting)),
            ("invokerHttpTime", difftime(self.InvokerGetResponseResult, self.InvokerSendArgument)),
            ("CollectLog", difftime(self.InvokerCollectLogEnd, self.InvokerCollectLogStart)),
            ("InvokerRecordingActivation", difftime(self.InvokerSendKafkaCompleteEnd, self.InvokerRecordingActivation)),
            ("SavetoDB", difftime(self.InvokerSendKafkaResultStart, self.InvokerSaveDB)),
            ("postKafkaResult", difftime(self.InvokerSendKafkaResultEnd, self.InvokerSendKafkaResultStart)),
            ("postKafkaComplete", difftime(self.InvokerSendKafkaCompleteEnd, self.InvokerSendKafkaCompleteStart))
        ]

mode = sys.argv[2]

notOKtid = re.compile("^tid_[0-9A-Za-z]+")

with open(sys.argv[1]) as fp:
    lines = fp.readlines()

    datalist = {}
    for line in lines:
        try:
            l = log(line.split())
            if notOKtid.match(l.tid):
                continue

            if l.tid not in datalist:
                datalist[l.tid] = request()

            if l:
                if mode == "controller":
                    datalist[l.tid].add_controller(l)
                elif mode == "invoker":
                    datalist[l.tid].add_invoker(l)

        except Exception as e:
            continue

    datalist = {k: v for k, v in datalist.items() if re.match(".*[0-9]+.*", k)}

    print(len(datalist))

    count = 0
    for v in datalist.values():
        if mode == "controller":
            try:
                m = len(v.diffarray_controller())
            except Exception as e:
                continue

        elif mode == "invoker":
            try:
                l = v.diffarray_invoker()
                m = len(l)
            except Exception as e:
                continue
        break

    for i in range(0, m):
        arr = []
        for v in datalist.values():
            if v.post != None and v.finish != None:
                if mode == "controller":
                    try:
                        x = v.diffarray_controller()
                        if x[i][1] != None:
                            arr.append(x[i])
                    except Exception as e:
                        continue
            elif mode == "invoker":
                try:
                    x = v.diffarray_invoker()
#                    if i == 0:
#                        if x[3] != None and x[i] != None:
#                            arr.append(x[i])
#                        continue

                    if x[i][1] != None:
                        arr.append(x[i])
                except Exception as e:
                    continue

        try:
            print(arr[0][0], statistics.mean([y[1] for y in arr]))
        except Exception as e:
            continue
