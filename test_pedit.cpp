#include "pedit.hpp"

#include <boost/filesystem/path.hpp>

namespace
{   // anonymous namespace
    bool InitBoostFilesystem()
    {
        boost::filesystem::path::default_name_check( boost::filesystem::no_check );
        return true; // a dummy value
    }
    static bool dummy = InitBoostFilesystem();
}

IMPLEMENT_APP( Pedit )
