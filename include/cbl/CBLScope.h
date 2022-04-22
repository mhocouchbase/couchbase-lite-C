//
//  CBLScope.h
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

CBL_CAPI_BEGIN

/** \defgroup scope   Scoe
    @{
    A \ref CBLCollection is a container for documents.
 */

// Not Refcounted:
CBL_REFCOUNTED(CBLScope*, Scope);

#ifdef __APPLE__
#pragma mark - SCOPE ACCESSORS
#endif
/** \name  Collection Accessors
    @{
    Getting information about a collection.
 */

/** Returns the scope name. */
FLString CBLScope_Name(const CBLScope* scope) CBLAPI;

/** Returns the names of the collections in the scope. */
FLMutableArray CBLScope_CollectionNames(const CBLScope* scope) CBLAPI;

/** Returns an existing collection in the scope. */
_cbl_warn_unused
CBLCollection* _cbl_nullable CBLScope_Collection(const CBLScope* scope, FLString name) CBLAPI;

/** @} */


#ifdef __APPLE__
#pragma mark - QUERY FACTORY
#endif
/** \name  Query Factory
    @{
 */

/** Create a query object within the scope context. */
_cbl_warn_unused
CBLQuery* _cbl_nullable CBLScope_CreateQuery(const CBLScope* scope,
                                             CBLQueryLanguage language,
                                             FLString queryString,
                                             int* _cbl_nullable outErrorPos,
                                             CBLError* _cbl_nullable outError) CBLAPI;

//Scope Context = scope2 XX
// _, _default, <db-name>
// SELECT * from scope1.collectionA joins collectionB XX
// SELECT * from scope1.collectionA joins scope2.collectionB

/** @} */
/** @} */    // end of outer \defgroup


#ifdef __APPLE__
#pragma mark - CHANGE LISTENER
#endif
/** \name  Change Listener
    @{
 */

/** Add a collection change listener to listener to changes in any collections in the scope. */
_cbl_warn_unused
CBLListenerToken* CBLScope_AddChangeListener(const CBLScope* scope,
                                             CBLCollectionChangeListener listener,
                                             void* _cbl_nullable context) CBLAPI;
/** @} */
/** @} */    // end of outer \defgroup

CBL_CAPI_END
