//
//  CBLCollection.h
//
// Copyright (c) 2022 Couchbase, Inc All rights reserved.
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
#include "CBLDocument.h"
#include "CBLQuery.h"
#include "fleece/Fleece.h"

CBL_CAPI_BEGIN

/** \defgroup collection   Collection
    @{
    A \ref CBLCollection is a container for documents.
 */

// No need to make CBLCollection refcounted:
CBL_REFCOUNTED(CBLCollection*, Collection);

/** \name  Scope and Collection Management
    @{
    Scope and Collection Management
 */

/** Returns the names of all scopes in the database. */
FLMutableArray CBLDatabase_ScopeNames(const CBLDatabase*) CBLAPI;

/** Returns the names of the collections in the specified scope */
FLMutableArray CBLDatabase_CollectionNames(const CBLDatabase*, FLString scopeName) CBLAPI;

/** Returns an existing scope.
    @NOTE: Internal pointer retained by the database object.
 */
_cbl_warn_unused
CBLScope* _cbl_nullable CBLDatabase_Scope(const CBLDatabase*, FLString name) CBLAPI;

/** Returns an existing collection. The default scope will be used if the given scopeName is a nullslice.
    @NOTE: Internal pointer retained by the database object.
 */
_cbl_warn_unused
CBLCollection* _cbl_nullable CBLDatabase_Collection(const CBLDatabase*,
                                                    FLString name,
                                                    FLString scopeName) CBLAPI;

/** Create a new collection. The default scope will be used if the given scopeName is a nullslice. */
_cbl_warn_unused
CBLCollection* _cbl_nullable CBLDatabase_CreateCollection(const CBLDatabase*,
                                                          FLString name,
                                                          FLString scopeName,
                                                          CBLError* outError) CBLAPI;

/** Delete a collection of the given name and scope. The default scope will be used if the given scopeName is a nullslice. */
bool CBLDatabase_DeleteCollection(const CBLDatabase*,
                                  FLString name,
                                  FLString scopeName,
                                  CBLError* outError) CBLAPI;

/** Returns the default scope. */
_cbl_warn_unused
CBLScope* CBLDatabase_DefaultScope(const CBLDatabase*) CBLAPI;

/** Returns the default collection. */
_cbl_warn_unused
CBLCollection* _cbl_nullable CBLDatabase_DefaultCollection(const CBLDatabase*) CBLAPI;

/** @} */

/** \name  Collection Accessors
    @{
    Getting information about a collection.
 */

/** Returns the scope of the collection. */
CBLScope* CBLCollection_Scope(const CBLCollection*) CBLAPI;

/** Returns the collection name. */
FLString CBLCollection_Name(const CBLCollection*) CBLAPI;

/** Returns the number of documents in the collection. */
uint64_t CBLCollection_Count(const CBLCollection*) CBLAPI;

/** @} */

/** \name  Document Lifecycle
    @{
    Document Lifecycle functions
 */

/** Reads a document from the collection, creating a new (immutable) \ref CBLDocument object.
    Each call to this function creates a new object (which must later be released.)
    @note  If you are reading the document in order to make changes to it, call
            \ref CBLCollection_GetMutableDocument instead.
    @param collection  The collection.
    @param docID  The ID of the document.
    @param outError  On failure, the error will be written here. (A nonexistent document is not
                    considered a failure; in that event the error code will be zero.)
    @return  A new \ref CBLDocument instance, or NULL if the doc doesn't exist or an error occurred. */
_cbl_warn_unused
const CBLDocument* _cbl_nullable CBLCollection_GetDocument(const CBLCollection* collection,
                                                           FLString docID,
                                                           CBLError* _cbl_nullable outError) CBLAPI;

/** Saves a (mutable) document to the collection.
    \warning If a newer revision has been saved since \p doc was loaded, it will be overwritten by
            this one. This can lead to data loss! To avoid this, call
            \ref CBLCollection_SaveDocumentWithConcurrencyControl or
            \ref CBLCollection_SaveDocumentWithConflictHandler instead.
    @param collection  The collection to save to.
    @param doc  The mutable document to save.
    @param outError  On failure, the error will be written here.
    @return  True on success, false on failure. */
bool CBLCollection_SaveDocument(CBLCollection* collection,
                                CBLDocument* doc,
                                CBLError* _cbl_nullable outError) CBLAPI;

/** Saves a (mutable) document to the collection.
    If a conflicting revision has been saved since \p doc was loaded, the \p concurrency
    parameter specifies whether the save should fail, or the conflicting revision should
    be overwritten with the revision being saved.
    If you need finer-grained control, call \ref CBLCollection_SaveDocumentWithConflictHandler instead.
    @param collection  The collection to save to.
    @param doc  The mutable document to save.
    @param concurrency  Conflict-handling strategy (fail or overwrite).
    @param outError  On failure, the error will be written here.
    @return  True on success, false on failure. */
bool CBLCollection_SaveDocumentWithConcurrencyControl(CBLCollection* collection,
                                                      CBLDocument* doc,
                                                      CBLConcurrencyControl concurrency,
                                                      CBLError* _cbl_nullable outError) CBLAPI;

/** Saves a (mutable) document to the collection, allowing for custom conflict handling in the event
    that the document has been updated since \p doc was loaded.
    @param collection  The collection to save to.
    @param doc  The mutable document to save.
    @param conflictHandler  The callback to be invoked if there is a conflict.
    @param context  An arbitrary value to be passed to the \p conflictHandler.
    @param outError  On failure, the error will be written here.
    @return  True on success, false on failure. */
bool CBLCollection_SaveDocumentWithConflictHandler(CBLCollection* collection,
                                                   CBLDocument* doc,
                                                   CBLConflictHandler conflictHandler,
                                                   void* _cbl_nullable context,
                                                   CBLError* _cbl_nullable outError) CBLAPI;

/** Deletes a document from the collection. Deletions are replicated.
    @warning  You are still responsible for releasing the CBLDocument.
    @param collection  The collection containing the document.
    @param document  The document to delete.
    @param outError  On failure, the error will be written here.
    @return  True if the document was deleted, false if an error occurred. */
bool CBLCollection_DeleteDocument(CBLCollection *collection,
                                  const CBLDocument* document,
                                  CBLError* _cbl_nullable outError) CBLAPI;

/** Deletes a document from the collection. Deletions are replicated.
    @warning  You are still responsible for releasing the CBLDocument.
    @param collection  The collection containing the document.
    @param document  The document to delete.
    @param concurrency  Conflict-handling strategy.
    @param outError  On failure, the error will be written here.
    @return  True if the document was deleted, false if an error occurred. */
bool CBLCollection_DeleteDocumentWithConcurrencyControl(CBLCollection *collection,
                                                        const CBLDocument* document,
                                                        CBLConcurrencyControl concurrency,
                                                        CBLError* _cbl_nullable outError) CBLAPI;

/** Purges a document. This removes all traces of the document from the collection.
    Purges are _not_ replicated. If the document is changed on a server, it will be re-created
    when pulled.
    @warning  You are still responsible for releasing the \ref CBLDocument reference.
    @note If you don't have the document in memory already, \ref CBLCollection_PurgeDocumentByID is a
          simpler shortcut.
    @param collection  The collection containing the document.
    @param document  The document to delete.
    @param outError  On failure, the error will be written here.
    @return  True if the document was purged, false if it doesn't exist or the purge failed. */
bool CBLCollection_PurgeDocument(CBLCollection* collection,
                                 const CBLDocument* document,
                                 CBLError* _cbl_nullable outError) CBLAPI;

/** Purges a document, given only its ID.
    @note  If no document with that ID exists, this function will return false but the error
            code will be zero.
    @param collection  The collection.
    @param docID  The document ID to purge.
    @param outError  On failure, the error will be written here.
    @return  True if the document was purged, false if it doesn't exist or the purge failed.
 */
bool CBLCollection_PurgeDocumentByID(CBLCollection* collection,
                                     FLString docID,
                                     CBLError* _cbl_nullable outError) CBLAPI;

/** @} */


/** \name  Mutable documents
    @{
    The type `CBLDocument*` without a `const` qualifier refers to a _mutable_ document instance.
    A mutable document exposes its properties as a mutable dictionary, so you can change them
    in place and then call \ref CBLCollection_SaveDocument to persist the changes.
 */

/** Reads a document from the collection, in mutable form that can be updated and saved.
    (This function is otherwise identical to \ref CBLCollection_GetDocument.)
    @note  You must release the document when you're done with it.
    @param collection  The collection.
    @param docID  The ID of the document.
    @param outError  On failure, the error will be written here. (A nonexistent document is not
                    considered a failure; in that event the error code will be zero.)
    @return  A new \ref CBLDocument instance, or NULL if the doc doesn't exist or an error occurred. */
_cbl_warn_unused
CBLDocument* _cbl_nullable CBLCollection_GetMutableDocument(CBLCollection* collection,
                                                            FLString docID,
                                                            CBLError* _cbl_nullable outError) CBLAPI;

/** @} */

/** \name  Document Expiry
    @{
*/

/** Returns the time, if any, at which a given document will expire and be purged.
    Documents don't normally expire; you have to call \ref CBLCollection_SetDocumentExpiration
    to set a document's expiration time.
    @param collection  The collection.
    @param docID  The ID of the document.
    @param outError  On failure, an error is written here.
    @return  The expiration time as a CBLTimestamp (milliseconds since Unix epoch),
             or 0 if the document does not have an expiration,
             or -1 if the call failed. */
CBLTimestamp CBLCollection_GetDocumentExpiration(CBLCollection* collection,
                                                 FLSlice docID,
                                                 CBLError* _cbl_nullable outError) CBLAPI;

/** Sets or clears the expiration time of a document.
    @param collection  The collection.
    @param docID  The ID of the document.
    @param expiration  The expiration time as a CBLTimestamp (milliseconds since Unix epoch),
                        or 0 if the document should never expire.
    @param outError  On failure, an error is written here.
    @return  True on success, false on failure. */
bool CBLCollection_SetDocumentExpiration(CBLCollection* collection,
                                         FLSlice docID,
                                         CBLTimestamp expiration,
                                         CBLError* _cbl_nullable outError) CBLAPI;
/** @} */


/** \name  Query Index
    @{
 */

/** Creates a value index.
    Indexes are persistent.
    If an identical index with that name already exists, nothing happens (and no error is returned.)
    If a non-identical index with that name already exists, it is deleted and re-created. */
bool CBLCollection_CreateValueIndex(CBLCollection* collection,
                                    FLString name,
                                    CBLValueIndexConfiguration config,
                                    CBLError* _cbl_nullable outError) CBLAPI;
 
/** Creates a full-text index.
    Indexes are persistent.
    If an identical index with that name already exists, nothing happens (and no error is returned.)
    If a non-identical index with that name already exists, it is deleted and re-created. */
bool CBLCollection_CreateFullTextIndex(CBLCollection* collection,
                                       FLString name,
                                       CBLFullTextIndexConfiguration config,
                                       CBLError* _cbl_nullable outError) CBLAPI;

/** Deletes an index given its name. */
bool CBLCollection_DeleteIndex(CBLCollection* collection,
                               FLString name,
                               CBLError* _cbl_nullable outError) CBLAPI;

/** Returns the names of the indexes on this collection, as a Fleece array of strings.
    @note  You are responsible for releasing the returned Fleece array. */
_cbl_warn_unused
FLArray CBLCollection_GetIndexNames(CBLCollection *db) CBLAPI;
/** @} */


/** \name  Change Listeners
    @{
    A collection change listener lets you detect changes made to all documents in a collection.
    (If you only want to observe specific documents, use a \ref CBLDocumentCollectionChangeListener instead.)
    @note If there are multiple \ref CBLCollection instances on the same database file, each one's
    listeners will be notified of changes made by other collection instances.
    @warning  Changes made to the database file by other processes will _not_ be notified. */

typedef struct {
    const CBLCollection* collection;    ///< The collection that changed.
    size_t numDocs;                     ///< The number of documents.
    FLString *docIDs;                   ///<The IDs of the documents that changed.
} CBLCollectionChange;

    
/** A collection change listener callback, invoked after one or more documents are changed on disk.
 @warning   By default, this listener may be called on arbitrary threads. If your code isn't
            prepared for that, you may want to use \ref CBLDatabase_BufferNotifications
            so that listeners will be called in a safe context.
 @param context  An arbitrary value given when the callback was registered.
 @param change The collection change information. */
typedef void (*CBLCollectionChangeListener)(void* _cbl_nullable context,
                                            const CBLCollectionChange* change);

/** Registers a collection change listener callback. It will be called after one or more
    documents are changed on disk.
    @param collection  The collection to observe.
    @param listener  The callback to be invoked.
    @param context  An opaque value that will be passed to the callback.
    @return  A token to be passed to \ref CBLListener_Remove when it's time to remove the listener.*/
_cbl_warn_unused
CBLListenerToken* CBLCollection_AddChangeListener(const CBLCollection* collection,
                                                  CBLCollectionChangeListener listener,
                                                  void* _cbl_nullable context) CBLAPI;

/** @} */


#ifdef __APPLE__
#pragma mark - DOCUMENT LISTENERS
#endif
/** \name  Document listeners
    @{
    A document change listener lets you detect changes made to a specific document after they
    are persisted to the collection.
    @note If there are multiple CBLCollection instances on the same database file, each one's
    document listeners will be notified of changes made by other collection instances.
 */

typedef struct {
    const CBLCollection* collection;    ///< The collection that changed.
    FLString docID;                     ///<The document's ID.
} CBLDocumentChange;

/** A document change listener callback, invoked after a specific document is changed on disk.
    @warning  By default, this listener may be called on arbitrary threads. If your code isn't
                    prepared for that, you may want to use \ref CBLDatabase_BufferNotifications
                    so that listeners will be called in a safe context.
    @param context  An arbitrary value given when the callback was registered.
    @param change  The document change info. */
typedef void (*CBLCollectionDocumentChangeListener)(void *context,
                                                    const CBLDocumentChange* change);

/** Registers a document change listener callback. It will be called after a specific document
    is changed on disk.
    @param collection  The collection to observe.
    @param docID  The ID of the document to observe.
    @param listener  The callback to be invoked.
    @param context  An opaque value that will be passed to the callback.
    @return  A token to be passed to \ref CBLListener_Remove when it's time to remove the
            listener.*/
_cbl_warn_unused
CBLListenerToken* CBLCollection_AddDocumentChangeListener(const CBLCollection* collection,
                                                          FLString docID,
                                                          CBLCollectionDocumentChangeListener listener,
                                                          void* _cbl_nullable context) CBLAPI;

/** @} */
/** @} */    // end of outer \defgroup

CBL_CAPI_END
