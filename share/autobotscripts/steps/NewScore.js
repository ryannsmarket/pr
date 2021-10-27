/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

module.exports = {
    openNewScoreDialog: function()
    {
        api.autobot.async(function() {
            api.navigation.triggerControl("RecentScores", "RecentScores", "New score")
        })
    },

    done: function()
    {
        api.navigation.triggerControl("NewScoreDialog", "BottomPanel", "Done")
    },

    сhooseInstrument: function(family, instrument)
    {
        api.navigation.goToControl("NewScoreDialog", "FamilyView", family)
        api.navigation.goToControl("NewScoreDialog", "InstrumentsView", instrument)
        api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")
    },

    chooseRandomInstruments: function(count, see_msec)
    {
        see_msec = see_msec || 50
        api.log.debug("chooseRandomInstruments count: " + count)
        for (var i = 0; i < count; i++) {

            api.log.debug("chooseRandomInstruments i: " + i)
            // Go to first family
            api.navigation.goToControl("NewScoreDialog", "FamilyView", "Woodwinds")

            // Choose family
            var familyCount = api.autobot.randomInt(0, 20);
            api.log.debug("chooseRandomInstruments familyCount: " + familyCount)
            for (var f = 0; f < familyCount; f++) {
                api.navigation.down()
                api.autobot.seeChanges(see_msec)
            }

            if (api.navigation.activeControl() === "genreBox") {
                api.navigation.down()
            }

            api.context.setStepVal("family_" + i, api.navigation.activeControl())

            // Got to Instruments
            api.navigation.nextPanel()
            api.autobot.seeChanges(see_msec)

            // Choose instrument
            var instrCount = api.autobot.randomInt(0, 20);
            api.log.debug("chooseRandomInstruments instrCount: " + instrCount)
            for (var j = 0; j < instrCount; j++) {
                api.navigation.down()
                api.autobot.seeChanges(see_msec)
            }

            if (api.navigation.activeControl() === "SearchInstruments") {
                api.navigation.down()
            }

            api.context.setStepVal("instrument_" + i, api.navigation.activeControl())

            // Select
            api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")
        }

        // Done
        api.navigation.triggerControl("NewScoreDialog", "BottomPanel", "Done")
    },

    selectTab: function(tab)
    {
        switch (tab) {
        case "instruments":
            api.navigation.triggerControl("NewScoreDialog", "ChooseTabPanel", "Choose instruments")
            break;
        case "templates":
            api.navigation.triggerControl("NewScoreDialog", "ChooseTabPanel", "Choose from template")
            break;
        default:
            api.log.error("unknown tab: " + tab)
        }
    },

    chooseTemplate: function(category, template)
    {
        api.navigation.goToControl("NewScoreDialog", "Category", category)
        api.navigation.goToControl("NewScoreDialog", "Template", template)
    }
}


