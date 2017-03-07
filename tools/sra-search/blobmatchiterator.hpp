/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef _hpp_blob_match_iterator_
#define _hpp_blob_match_iterator_

#include <ngs-vdb/inc/VdbReadCollection.hpp>

#include "matchiterator.hpp"

struct KLock;

// Searches blob by blob
class BlobMatchIterator : public MatchIterator
{
public:
    BlobMatchIterator ( SearchBlock :: Factory& p_factory, const std::string& p_accession );

    virtual ~BlobMatchIterator ();

    virtual SearchBuffer* NextBuffer ();

private:
    ncbi::ngs::vdb::VdbReadCollection       m_coll;
    KLock*                                  m_accessionLock;
    ncbi::ngs::vdb::FragmentBlobIterator    m_blobIt;
};

#endif
