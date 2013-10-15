/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Config.h"
#include "Errors.h"

// Defined here as it must not be exposed to end-users.
bool ConfigMgr::GetValueHelper(const char* strName, ACE_TString &Result)
{
    GuardType guard(m_configLock);

    if (m_config.get() == 0)
        return false;

    ACE_TString section_name;
    ACE_Configuration_Section_Key section_key;
    const ACE_Configuration_Section_Key &root_key = m_config->root_section();

    int i = 0;
    while (m_config->enumerate_sections(root_key, i, section_name) == 0)
    {
        m_config->open_section(root_key, section_name.c_str(), 0, section_key);
        if (m_config->get_string_value(section_key, strName, Result) == 0)
            return true;
        ++i;
    }

    return false;
}

bool ConfigMgr::LoadInitial(char const* szFile)
{
    ASSERT(szFile);

    GuardType guard(m_configLock);

    m_filename = szFile;
    m_config.reset(new ACE_Configuration_Heap());
    if (m_config->open() == 0)
        if (LoadData(m_filename.c_str()))
            return true;

    m_config.reset();
    return false;
}

bool ConfigMgr::LoadMore(char const* szFile)
{
    ASSERT(szFile);
    ASSERT(m_config);

    GuardType guard(m_configLock);

    return LoadData(szFile);
}

bool ConfigMgr::Reload()
{
    return LoadInitial(m_filename.c_str());
}

bool ConfigMgr::LoadData(char const* szFile)
{
    ACE_Ini_ImpExp config_importer(*m_config.get());
    if (config_importer.import_config(szFile) == 0)
        return true;

    return false;
}

std::string ConfigMgr::GetStringDefault(const char* szName, const std::string &def)
{
    ACE_TString val;
    return GetValueHelper(szName, val) ? val.c_str() : def;
}

bool ConfigMgr::GetBoolDefault(const char* strName, bool def)
{
    ACE_TString val;

    if (!GetValueHelper(strName, val))
        return def;

    return 
		(val == "true" || val == "TRUE" || val == "yes" || val == "YES" || val == "1");
}

int ConfigMgr::GetIntDefault(const char* strName, int def)
{
    ACE_TString val;
    return GetValueHelper(strName, val) ? atoi(val.c_str()) : def;
}

float ConfigMgr::GetFloatDefault(const char* strName, float def)
{
    ACE_TString val;
    return GetValueHelper(strName, val) ? (float)atof(val.c_str()) : def;
}

std::string const& ConfigMgr::GetFilename()
{
    GuardType guard(m_configLock);
    return m_filename;
}
