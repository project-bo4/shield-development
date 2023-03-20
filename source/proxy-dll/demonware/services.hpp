#pragma once

#include "bit_buffer.hpp"
#include "byte_buffer.hpp"
#include "data_types.hpp"
#include "reply.hpp"
#include "service.hpp"
#include "servers/service_server.hpp"

#include "services/bdStats.hpp"             //   4 [  UNKNOWN  ]
#include "services/bdProfiles.hpp"          //   8
#include "services/bdTitleUtilities.hpp"    //  12 [ ESSENTIAL ]
#include "services/bdKeyArchive.hpp"        //  15 [  USELESS  ]
#include "services/bdBandwidthTest.hpp"     //  18
#include "services/bdCounter.hpp"           //  23
#include "services/bdDML.hpp"               //  27 [ ESSENTIAL ]
#include "services/bdGroup.hpp"             //  28 [  USELESS  ]
#include "services/bdAnticheat.hpp"         //  38 [  UNKNOWN  ]
#include "services/bdTags.hpp"              //  52
#include "services/bdPooledStorage.hpp"     //  58
#include "services/bdEventLog.hpp"          //  67
#include "services/bdRichPresence.hpp"      //  68
#include "services/bdMarketplace.hpp"       //  80
#include "services/bdPublisherVariables.hpp"//  95
#include "services/bdMarketingComms.hpp"    // 104
#include "services/bdUNK125.hpp"            // 125 [ PATCHED OUT ]
#include "services/bdObjectStore.hpp"       // 193 [  ESSENTIAL  ]
#include "services/bdLootGeneration.hpp"    // 195 [   UNKNOWN   ]