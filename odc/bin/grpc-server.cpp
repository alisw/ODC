/********************************************************************************
 * Copyright (C) 2019-2022 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH  *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *              GNU Lesser General Public Licence (LGPL) version 3,             *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

// ODC
#include <odc/BuildConstants.h>
#include <odc/CliHelper.h>
#include <odc/Logger.h>
#include <odc/MiscUtils.h>
#include <odc/Version.h>
#include <odc/grpc/AsyncController.h>
#include <odc/grpc/SyncController.h>
// STD
#include <cstdlib>
#include <iostream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// DDS
#include <dds/Tools.h>

using namespace std;
using namespace odc::core;
namespace bpo = boost::program_options;

int main(int argc, char** argv)
{
    try {
        bool sync;
        size_t timeout;
        string host;
        CLogger::SConfig logConfig;
        CPluginManager::PluginMap_t pluginMap;
        CPluginManager::PluginMap_t triggerMap;
        string restoreId;

        bpo::options_description options("dds-control-server options");
        CliHelper::addHelpOptions(options);
        CliHelper::addVersionOptions(options);
        CliHelper::addSyncOptions(options, sync);
        CliHelper::addTimeoutOptions(options, timeout);
        CliHelper::addHostOptions(options, host);
        CliHelper::addLogOptions(options, logConfig);
        CliHelper::addResourcePluginOptions(options, pluginMap);
        CliHelper::addRequestTriggersOptions(options, triggerMap);
        CliHelper::addRestoreOptions(options, restoreId);

        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        try {
            CLogger::instance().init(logConfig);
        } catch (exception& _e) {
            cerr << "Can't initialize log: " << _e.what() << endl;
            return EXIT_FAILURE;
        }

        if (vm.count("help")) {
            OLOG(clean) << options;
            return EXIT_SUCCESS;
        }

        if (vm.count("version")) {
            OLOG(clean) << ODC_VERSION;
            return EXIT_SUCCESS;
        }

        setupGrpcVerbosity(logConfig.m_severity);

        CliHelper::parsePluginMapOptions(vm, pluginMap, "rp");
        CliHelper::parsePluginMapOptions(vm, triggerMap, "rt");

        if (sync) {
            odc::grpc::SyncController ctrl;
            ctrl.setTimeout(chrono::seconds(timeout));
            ctrl.registerResourcePlugins(pluginMap);
            ctrl.registerRequestTriggers(triggerMap);
            if (!restoreId.empty()) {
                ctrl.restore(restoreId);
            }
            ctrl.run(host);
        } else {
            odc::grpc::AsyncController ctrl;
            ctrl.setTimeout(chrono::seconds(timeout));
            ctrl.registerResourcePlugins(pluginMap);
            ctrl.registerRequestTriggers(triggerMap);
            if (!restoreId.empty()) {
                ctrl.restore(restoreId);
            }
            ctrl.run(host);
        }
    } catch (exception& _e) {
        OLOG(clean) << _e.what();
        OLOG(fatal) << _e.what();
        return EXIT_FAILURE;
    } catch (...) {
        OLOG(clean) << "Unexpected Exception occurred.";
        OLOG(fatal) << "Unexpected Exception occurred.";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
