//
// CBLReplicator.h
//
// Copyright (c) 2018 Couchbase, Inc All rights reserved.
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

#pragma once
#include "CBLBase.h"
#include "fleece/Fleece.h"

CBL_CAPI_BEGIN

/** \defgroup replication   Replication
    A replicator is a background task that synchronizes changes between a local database and
    another database on a remote server (or on a peer device, or even another local database.)
    @{ */

/** \name  Configuration
    @{ */

/** The name of the HTTP cookie used by Sync Gateway to store session keys. */
CBL_PUBLIC extern const FLString kCBLAuthDefaultCookieName;

/** An opaque object representing the location of a database to replicate with. */
typedef struct CBLEndpoint CBLEndpoint;

/** Creates a new endpoint representing a server-based database at the given URL.
    The URL's scheme must be `ws` or `wss`, it must of course have a valid hostname,
    and its path must be the name of the database on that server.
    
    The port can be omitted; it defaults to 80 for `ws` and 443 for `wss`.
    For example: `wss://example.org/dbname`.
 
    If an invalid endpoint URL is specified, an error will be returned.
    */
_cbl_warn_unused
CBLEndpoint* _cbl_nullable CBLEndpoint_CreateWithURL(FLString url,
                                                     CBLError* _cbl_nullable outError) CBLAPI;

#ifdef COUCHBASE_ENTERPRISE
/** Creates a new endpoint representing another local database. (Enterprise Edition only.) */
_cbl_warn_unused
CBLEndpoint* CBLEndpoint_CreateWithLocalDB(CBLDatabase*) CBLAPI;
#endif

/** Frees a CBLEndpoint object. */
void CBLEndpoint_Free(CBLEndpoint* _cbl_nullable) CBLAPI;


/** An opaque object representing authentication credentials for a remote server. */
typedef struct CBLAuthenticator CBLAuthenticator;

/** Creates an authenticator for HTTP Basic (username/password) auth. */
_cbl_warn_unused
CBLAuthenticator* CBLAuth_CreatePassword(FLString username, FLString password) CBLAPI;

/** Creates an authenticator using a Couchbase Sync Gateway login session identifier,
    and optionally a cookie name (pass NULL for the default.) */
_cbl_warn_unused
CBLAuthenticator* CBLAuth_CreateSession(FLString sessionID, FLString cookieName) CBLAPI;

/** Frees a CBLAuthenticator object. */
void CBLAuth_Free(CBLAuthenticator* _cbl_nullable) CBLAPI;


/** Direction of replication: push, pull, or both. */
typedef CBL_ENUM(uint8_t, CBLReplicatorType) {
    kCBLReplicatorTypePushAndPull = 0,    ///< Bidirectional; both push and pull
    kCBLReplicatorTypePush,               ///< Pushing changes to the target
    kCBLReplicatorTypePull                ///< Pulling changes from the target
};


/** Flags describing a replicated document. */
typedef CBL_OPTIONS(unsigned, CBLDocumentFlags) {
    kCBLDocumentFlagsDeleted        = 1 << 0,   ///< The document has been deleted.
    kCBLDocumentFlagsAccessRemoved  = 1 << 1    ///< Lost access to the document on the server.
};


/** A callback that can decide whether a particular document should be pushed or pulled.
    @warning  This callback will be called on a background thread managed by the replicator.
                It must pay attention to thread-safety. It should not take a long time to return,
                or it will slow down the replicator.
    @param context  The `context` field of the \ref CBLReplicatorConfiguration.
    @param document  The document in question.
    @param flags  Indicates whether the document was deleted or removed.
    @return  True if the document should be replicated, false to skip it. */
typedef bool (*CBLReplicationFilter)(void* _cbl_nullable context,
                                     CBLDocument* document,
                                     CBLDocumentFlags flags);

/** Conflict-resolution callback for use in replications. This callback will be invoked
    when the replicator finds a newer server-side revision of a document that also has local
    changes. The local and remote changes must be resolved before the document can be pushed
    to the server.
    @note  Any new CBLBlob objects set to the resolved document returned by the callback must
            not be released. They need to be retained for installation while the resolved document
            is being saved into the database, and the replicator will be responsible for
            releasing them after they are installed.
    @warning  This callback will be called on a background thread managed by the replicator.
                It must pay attention to thread-safety. However, unlike a filter callback,
                it does not need to return quickly. If it needs to prompt for user input,
                that's OK.
    @param context  The `context` field of the \ref CBLReplicatorConfiguration.
    @param documentID  The ID of the conflicted document.
    @param localDocument  The current revision of the document in the local database,
                or NULL if the local document has been deleted.
    @param remoteDocument  The revision of the document found on the server,
                or NULL if the document has been deleted on the server.
    @return  The resolved document to save locally (and push, if the replicator is pushing.)
        This can be the same as \p localDocument or \p remoteDocument, or you can create
        a mutable copy of either one and modify it appropriately.
        Or return NULL if the resolution is to delete the document. */
typedef const CBLDocument* _cbl_nullable (*CBLConflictResolver)(void* _cbl_nullable context,
                                                  FLString documentID,
                                                  const CBLDocument* _cbl_nullable localDocument,
                                                  const CBLDocument* _cbl_nullable remoteDocument);

/** Default conflict resolver. This always returns `localDocument`. */
CBL_PUBLIC extern const CBLConflictResolver CBLDefaultConflictResolver;


/** Types of proxy servers, for CBLProxySettings. */
typedef CBL_ENUM(uint8_t, CBLProxyType) {
    kCBLProxyHTTP,                      ///< HTTP proxy; must support 'CONNECT' method
    kCBLProxyHTTPS,                     ///< HTTPS proxy; must support 'CONNECT' method
};


/** Proxy settings for the replicator. */
typedef struct {
    CBLProxyType type;                  ///< Type of proxy
    FLString hostname;               ///< Proxy server hostname or IP address
    uint16_t port;                      ///< Proxy server port
    FLString username;               ///< Username for proxy auth (optional)
    FLString password;               ///< Password for proxy auth
} CBLProxySettings;

#ifdef COUCHBASE_ENTERPRISE

/** Callback that encrypts \ref CBLEncryptable properties in documents of the default collection,
    pushed by the replicator.
    @note   If a null \ref FLSliceResult or an error is returned, the document will be failed to
            replicate with the \ref kCBLErrorCrypto error when. For security reason, the encryption
            cannot be skipped.
    @warning  <b>Deprecated :</b> Use CBLDocumentPropertyEncryptor instead. */
typedef FLSliceResult (*CBLPropertyEncryptor) (
    void* context,              ///< Replicator’s context
    FLString documentID,        ///< Document ID
    FLDict properties,          ///< Document properties
    FLString keyPath,           ///< Key path of the property to be encrypted
    FLSlice input,              ///< Property data to be encrypted
    FLStringResult* algorithm,  ///< On return: algorithm name (Optional: Default Value is 'CB_MOBILE_CUSTOM')
    FLStringResult* kid,        ///< On return: encryption key identifier (Optional)
    CBLError* error             ///< On return: error (Optional)
);

/** Callback that decrypts encrypted \ref CBLEncryptable properties in documents of the default collection,
    pulled by the replicator.
    @note   The decryption will be skipped (the encrypted data will be kept) when a null
            \ref FLSliceResult without an error is returned. If an error is returned,
            the document will be failed to replicate with the \ref kCBLErrorCrypto error.
    @warning  <b>Deprecated :</b> Use CBLDocumentPropertyDecryptor instead. */
typedef FLSliceResult (*CBLPropertyDecryptor) (
    void* context,              ///< Replicator’s context
    FLString documentID,        ///< Document ID
    FLDict properties,          ///< Document properties
    FLString keyPath,           ///< Key path of the property to be decrypted
    FLSlice input,              ///< Property data to be decrypted
    FLString algorithm,         ///< Algorithm name
    FLString kid,               ///< Encryption key identifier specified when encryting the value
    CBLError* error             ///< On return: error (Optional)
);

/** Callback that encrypts \ref CBLEncryptable properties in documents pushed by the replicator.
    \note   If a null \ref FLSliceResult or an error is returned, the document will be failed to
            replicate with the \ref kCBLErrorCrypto error when. For security reason, the encryption
            cannot be skipped. */
typedef FLSliceResult (*CBLDocumentPropertyEncryptor) (
    void* context,              ///< Replicator’s context
    FLString scope,             ///<Scope's name of the collection
    FLString collection,        ///<Collection's name
    FLString documentID,        ///< Document ID
    FLDict properties,          ///< Document properties
    FLString keyPath,           ///< Key path of the property to be encrypted
    FLSlice input,              ///< Property data to be encrypted
    FLStringResult* algorithm,  ///< On return: algorithm name (Optional: Default Value is 'CB_MOBILE_CUSTOM')
    FLStringResult* kid,        ///< On return: encryption key identifier (Optional)
    CBLError* error             ///< On return: error (Optional)
);

/** Callback that decrypts encrypted \ref CBLEncryptable properties in documents pulled by the replicator.
    \note   The decryption will be skipped (the encrypted data will be kept) when a null \ref FLSliceResult
            without an error is returned. If an error is returned, the document will be failed to replicate
            with the \ref kCBLErrorCrypto error. */
typedef FLSliceResult (*CBLDocumentPropertyDecryptor) (
    void* context,              ///< Replicator’s context
    FLString scope,             ///<Scope's name of the collection
    FLString collection,        ///<Collection's name
    FLString documentID,        ///< Document ID
    FLDict properties,          ///< Document properties
    FLString keyPath,           ///< Key path of the property to be decrypted
    FLSlice input,              ///< Property data to be decrypted
    FLString algorithm,         ///< Algorithm name
    FLString kid,               ///< Encryption key identifier specified when encryting the value
    CBLError* error             ///< On return: error (Optional)
);

#endif

/** The collection and the configuration that can be configured specifically for the replication. */
typedef struct {
    CBLCollection* collection;                          ///< The collection.
    
    CBLConflictResolver _cbl_nullable conflictResolver; ///< Optional conflict-resolver callback
    
    CBLReplicationFilter _cbl_nullable pushFilter;      ///< Optional callback to filter which docs are pushed
    CBLReplicationFilter _cbl_nullable pullFilter;      ///< Optional callback to validate incoming docs
    
    FLArray _cbl_nullable channels;                     ///< Optional set of channels to pull from
    FLArray _cbl_nullable documentIDs;                  ///< Optional set of document IDs to replicate
} CBLReplicationCollection;

/** The configuration of a replicator. */
typedef struct {
    /** The database to replicate
        @warning  <b>Deprecated :</b> Use collections instead. */
    CBLDatabase* database;
    CBLEndpoint* endpoint;                  ///< The address of the other database to replicate with
    
    //-- Types:
    CBLReplicatorType replicatorType;       ///< Push, pull or both
    bool continuous;                        ///< Continuous replication?
    
    //-- Auto Purge:
    /** If auto purge is active, then the library will automatically purge any documents that
        the replicating user loses access to via the Sync Function on Sync Gateway.
        If disableAutoPurge is true, this behavior is disabled and an access removed
        event will be sent to any document listeners that are active on the replicator.
     
        IMPORTANT: For performance reasons, the document listeners must be
        added *before* the replicator is started or they will not receive the events. */
    bool disableAutoPurge;
    
    //-- Retry Logic:
    unsigned maxAttempts;               ///< Max retry attempts where the initial connect to replicate counts toward the given value.
                                        ///< Specify 0 to use the default value, 10 times for a non-continuous replicator and max-int time for a continuous replicator. Specify 1 means there will be no retry after the first attempt.
    unsigned maxAttemptWaitTime;        ///< Max wait time between retry attempts in seconds. Specify 0 to use the default value of 300 seconds.
    //-- WebSocket:
    unsigned heartbeat;                 ///< The heartbeat interval in seconds. Specify 0 to use the default value of 300 seconds.
    FLString networkInterface;          ///< The specific network interface to be used by the replicator to connect to the remote server.
                                        ///< If not specified, an active network interface based on the OS's routing table will be used.
    
    //-- HTTP settings:
    CBLAuthenticator* _cbl_nullable authenticator;    ///< Authentication credentials, if needed
    const CBLProxySettings* _cbl_nullable proxy;      ///< HTTP client proxy settings
    FLDict _cbl_nullable headers;                     ///< Extra HTTP headers to add to the WebSocket request
    
    //-- TLS settings:
    FLSlice pinnedServerCertificate;    ///< An X.509 cert to "pin" TLS connections to (PEM or DER)
    FLSlice trustedRootCertificates;    ///< Set of anchor certs (PEM format)
    
    //-- Filtering:
    /** Optional set of channels to pull from
        @warning  <b>Deprecated :</b> Use CBLReplicationCollection.channels instead. */
    FLArray _cbl_nullable channels;
    
    /** Optional set of document IDs to replicate
        @warning  <b>Deprecated :</b> Use CBLReplicationCollection.documentIDs instead. */
    FLArray _cbl_nullable documentIDs;
    
    /** Optional callback to filter which docs are pushed.
        @warning  <b>Deprecated :</b> Use CBLReplicationCollection.pushFilter instead. */
    CBLReplicationFilter _cbl_nullable pushFilter;
    
    /** Optional callback to validate incoming docs.
        @warning  <b>Deprecated :</b> Use CBLReplicationCollection.pullFilter instead. */
    CBLReplicationFilter _cbl_nullable pullFilter;
    
    /** Optional conflict-resolver callback.
        @warning  <b>Deprecated :</b> Use CBLReplicationCollection.conflictResolver instead. */
    CBLConflictResolver _cbl_nullable conflictResolver;
    
    //-- Context:
    void* _cbl_nullable context;                      ///< Arbitrary value that will be passed to callbacks
    
#ifdef COUCHBASE_ENTERPRISE
    //-- Property Encryption
    /** Optional callback to encrypt \ref CBLEncryptable values of the documents in
        the default collection. If the default collection is not part of the replication,
        the replicator will fail to create with an error.
        @warning  <b>Deprecated :</b> Use documentPropertyEncryptor instead. */
    CBLPropertyEncryptor propertyEncryptor;
    
    /** Optional callback to decrypt encrypted \ref CBLEncryptable values of the documents in
        the default collection. If the default collection is not part of the replication,
        the replicator will fail to create with an error.
        @warning  <b>Deprecated :</b> Use documentPropertyDecryptor instead. */
    CBLPropertyDecryptor propertyDecryptor;
    
    CBLDocumentPropertyEncryptor documentPropertyEncryptor;   ///< Optional callback to encrypt \ref CBLEncryptable values.
    CBLDocumentPropertyDecryptor documentPropertyDecryptor;   ///< Optional callback to decrypt encrypted \ref CBLEncryptable values.
#endif
    
    CBLReplicationCollection* collections;  ///< The collections to replicate with the target's endpoint
    size_t collectionCount;                 ///< The number of collections
} CBLReplicatorConfiguration;


/** @} */


/** \name  Lifecycle
    @{ */

CBL_REFCOUNTED(CBLReplicator*, Replicator);

/** Creates a replicator with the given configuration. */
_cbl_warn_unused
CBLReplicator* _cbl_nullable CBLReplicator_Create(const CBLReplicatorConfiguration*,
                                                  CBLError* _cbl_nullable outError) CBLAPI;

/** Returns the configuration of an existing replicator. */
const CBLReplicatorConfiguration* CBLReplicator_Config(CBLReplicator*) CBLAPI;

/** Starts a replicator, asynchronously. Does nothing if it's already started.
    @param replicator  The replicator instance.
    @param resetCheckpoint  If true, the persistent saved state ("checkpoint") for this replication
                        will be discarded, causing it to re-scan all documents. This significantly
                        increases time and bandwidth (redundant docs are not transferred, but their
                        IDs are) but can resolve unexpected problems with missing documents if one
                        side or the other has gotten out of sync. */
void CBLReplicator_Start(CBLReplicator *replicator,
                         bool resetCheckpoint) CBLAPI;

/** Stops a running replicator, asynchronously. Does nothing if it's not already started.
    The replicator will call your \ref CBLReplicatorChangeListener with an activity level of
    \ref kCBLReplicatorStopped after it stops. Until then, consider it still active. */
void CBLReplicator_Stop(CBLReplicator*) CBLAPI;

/** Informs the replicator whether it's considered possible to reach the remote host with
    the current network configuration. The default value is true. This only affects the
    replicator's behavior while it's in the Offline state:
    * Setting it to false will cancel any pending retry and prevent future automatic retries.
    * Setting it back to true will initiate an immediate retry.*/
void CBLReplicator_SetHostReachable(CBLReplicator*,
                                    bool reachable) CBLAPI;

/** Puts the replicator in or out of "suspended" state. The default is false.
    * Setting suspended=true causes the replicator to disconnect and enter Offline state;
      it will not attempt to reconnect while it's suspended.
    * Setting suspended=false causes the replicator to attempt to reconnect, _if_ it was
      connected when suspended, and is still in Offline state. */
void CBLReplicator_SetSuspended(CBLReplicator* repl, bool suspended) CBLAPI;

/** @} */


/** \name  Status and Progress
    @{
 */

/** The possible states a replicator can be in during its lifecycle. */
typedef CBL_ENUM(uint8_t, CBLReplicatorActivityLevel) {
    kCBLReplicatorStopped,    ///< The replicator is unstarted, finished, or hit a fatal error.
    kCBLReplicatorOffline,    ///< The replicator is offline, as the remote host is unreachable.
    kCBLReplicatorConnecting, ///< The replicator is connecting to the remote host.
    kCBLReplicatorIdle,       ///< The replicator is inactive, waiting for changes to sync.
    kCBLReplicatorBusy        ///< The replicator is actively transferring data.
};

/** A fractional progress value, ranging from 0.0 to 1.0 as replication progresses.
    The value is very approximate and may bounce around during replication; making it more
    accurate would require slowing down the replicator and incurring more load on the server.
    It's fine to use in a progress bar, though. */
typedef struct {
    float complete;             ///<Very-approximate fractional completion, from 0.0 to 1.0
    uint64_t documentCount;     ///< Number of documents transferred so far
} CBLReplicatorProgress;

/** A replicator's current status. */
typedef struct {
    CBLReplicatorActivityLevel activity;    ///< Current state
    CBLReplicatorProgress progress;         ///< Approximate fraction complete
    CBLError error;                         ///< Error, if any
} CBLReplicatorStatus;

/** Returns the replicator's current status. */
CBLReplicatorStatus CBLReplicator_Status(CBLReplicator*) CBLAPI;

/** Indicates which documents in the default collection have local changes that have not yet
    been pushed to the server by this replicator. This is of course a snapshot, that will
    go out of date as the replicator makes progress and/or documents are saved locally.

    The result is, effectively, a set of document IDs: a dictionary whose keys are the IDs and
    values are `true`.
    If there are no pending documents, the dictionary is empty.
    On error, NULL is returned.

    @note  This function can be called on a stopped or un-started replicator.
    @note  Documents that would never be pushed by this replicator, due to its configuration's
           `pushFilter` or `docIDs`, are ignored.
    @warning  You are responsible for releasing the returned array via \ref FLValue_Release.
    @warning  If the default collection is not part of the replication, a NULL with an error
              will be returned.
    @warning  <b>Deprecated :</b> Use CBLReplicator_PendingDocumentIDs2 instead. */
_cbl_warn_unused
FLDict _cbl_nullable CBLReplicator_PendingDocumentIDs(CBLReplicator*, CBLError* _cbl_nullable outError) CBLAPI;

/** Indicates whether the document in the default with the given ID has local changes that
    have not yet been pushed to the server by this replicator.

    This is equivalent to, but faster than, calling \ref CBLReplicator_PendingDocumentIDs and
    checking whether the result contains \p docID. See that function's documentation for details.

    @note  A `false` result means the document is not pending, _or_ there was an error.
           To tell the difference, compare the error code to zero.
    @warning  If the default collection is not part of the replication, a NULL with an error
              will be returned.
    @warning  <b>Deprecated :</b> Use CBLReplicator_IsDocumentPending2 instead. */
bool CBLReplicator_IsDocumentPending(CBLReplicator *repl,
                                     FLString docID,
                                     CBLError* _cbl_nullable outError) CBLAPI;

/** Indicates which documents in the given collection have local changes that have not yet been
    pushed to the server by this replicator. This is of course a snapshot, that will go out of date
    as the replicator makes progress and/or documents are saved locally.
    
    The result is, effectively, a set of document IDs: a dictionary whose keys are the IDs and
    values are `true`.
    If there are no pending documents, the dictionary is empty.
    On error, NULL is returned.
    @warning  If the given collection is not part of the replication, a NULL with an error will be returned. */
FLDict _cbl_nullable CBLReplicator_PendingDocumentIDs2(CBLReplicator*,
                                                       const CBLCollection* collection,
                                                       CBLError* _cbl_nullable outError) CBLAPI;

/** Indicates whether the document with the given ID in the given collection has local changes
    that have not yet been pushed to the server by this replicator.
 
    This is equivalent to, but faster than, calling \ref CBLReplicator_PendingDocumentIDs2 and
    checking whether the result contains \p docID. See that function's documentation for details.

    @note  A `false` result means the document is not pending, _or_ there was an error.
        To tell the difference, compare the error code to zero.
    @warning  If the given collection is not part of the replication, a NULL with an error will be returned. */
bool CBLReplicator_IsDocumentPending2(CBLReplicator *repl,
                                      FLString docID,
                                      const CBLCollection* collection,
                                      CBLError* _cbl_nullable outError) CBLAPI;

/** A callback that notifies you when the replicator's status changes.
    @warning  This callback will be called on a background thread managed by the replicator.
                It must pay attention to thread-safety. It should not take a long time to return,
                or it will slow down the replicator.
    @param context  The value given when the listener was added.
    @param replicator  The replicator.
    @param status  The replicator's status. */
typedef void (*CBLReplicatorChangeListener)(void* _cbl_nullable context,
                                            CBLReplicator *replicator,
                                            const CBLReplicatorStatus *status);

/** Adds a listener that will be called when the replicator's status changes. */
_cbl_warn_unused
CBLListenerToken* CBLReplicator_AddChangeListener(CBLReplicator*,
                                                  CBLReplicatorChangeListener,
                                                  void* _cbl_nullable context) CBLAPI;


/** Information about a document that's been pushed or pulled. */
typedef struct {
    FLString ID;                ///< The document ID.
    CBLDocumentFlags flags;     ///< Indicates whether the document was deleted or removed.
    CBLError error;             ///< If the code is nonzero, the document failed to replicate.
    FLString scope;             ///<The scope name of the collection
    FLString collection;        ///<The collection name.
} CBLReplicatedDocument;

/** A callback that notifies you when documents are replicated.
    @warning  This callback will be called on a background thread managed by the replicator.
                It must pay attention to thread-safety. It should not take a long time to return,
                or it will slow down the replicator.
    @param context  The value given when the listener was added.
    @param replicator  The replicator.
    @param isPush  True if the document(s) were pushed, false if pulled.
    @param numDocuments  The number of documents reported by this callback.
    @param documents  An array with information about each document. */
typedef void (*CBLDocumentReplicationListener)(void *context,
                                               CBLReplicator *replicator,
                                               bool isPush,
                                               unsigned numDocuments,
                                               const CBLReplicatedDocument* documents);

/** Adds a listener that will be called when documents are replicated. */
_cbl_warn_unused
CBLListenerToken* CBLReplicator_AddDocumentReplicationListener(CBLReplicator*,
                                                               CBLDocumentReplicationListener,
                                                               void* _cbl_nullable context) CBLAPI;

/** @} */
/** @} */

CBL_CAPI_END
