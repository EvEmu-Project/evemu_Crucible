using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common;
using EVEMon.Common.Threading;

namespace EVEMon.LogitechG15
{
    /// <summary>
    /// This class takes care to drive the <see cref="Lcdisplay"/> according to the settings, the characters, the global events, etc. 
    /// <para>It malaxes and fetches data to the <see cref="Lcdisplay"/> while this last one holds the responsibility to cycle between chars, 
    /// format the display, and update to the device.</para>
    /// </summary>
    public static class G15Handler
    {
        private static Lcdisplay s_lcd;
        private static bool s_running;
        private static bool s_startupError;

        private static Object s_syncLock = new Object();


        #region Initialize

        /// <summary>
        /// Initialises the G15 event handles
        /// </summary>
        public static void Initialize()
        {
            EveClient.TimerTick += EveClient_TimerTick;
            EveClient.QueuedSkillsCompleted += EveClient_QueuedSkillsCompleted;

            // Subscribe to events which occur of G15 buttons pressed
            Lcdisplay.APIUpdateRequested += Lcdisplay_APIUpdateRequested;
            Lcdisplay.AutoCycleChanged += Lcdisplay_AutoCycleChanged;
            Lcdisplay.CurrentCharacterChanged += Lcdisplay_CurrentCharacterChanged;
        }

        #endregion


        #region LCD Updater

        /// <summary>
        /// Update on every second (and when some of the G15 buttons are pressed)
        /// </summary>
        private static void UpdateOnTimerTick()
        {
            // No G15 keyboard connected
            if (s_startupError)
                return;

            // Did the state changed ?
            if (Settings.G15.Enabled != s_running)
            {
                if (!s_running)
                {
                    Start();
                    if (s_startupError)
                        return;
                }
                else
                {
                    Stop();
                }
            }

            // Run
            if (s_running)
            {
                UpdateG15Data();
                s_lcd.Paint();
            }
        }

        #endregion


        #region Helper Methods

        /// <summary>
        /// Starts the LCD display.
        /// </summary>
        private static void Start()
        {
            try
            {
                s_lcd = Lcdisplay.Instance();
                s_lcd.SwitchState(LcdState.SplashScreen);
                s_running = true;
            }
            catch(Exception ex)
            {
                EveClient.Trace(ex.Message);
                s_startupError = true;
                s_running = false;
            }
        }

        /// <summary>
        /// Stop the LCD display.
        /// </summary>
        private static void Stop()
        {
            try
            {
                s_lcd.Dispose();
            }
            catch (Exception ex)
            {
                EveClient.Trace(ex.Message);
            }
            finally
            {
                s_lcd = null;
                s_running = false;
            }
        }

        /// <summary>
        /// Update the display once every second
        /// </summary>
        private static void UpdateG15Data()
        {
            // Settings
            s_lcd.Cycle = Settings.G15.UseCharactersCycle;
            s_lcd.CycleInterval = Settings.G15.CharactersCycleInterval;
            s_lcd.CycleSkillQueueTime = Settings.G15.UseTimeFormatsCycle;
            s_lcd.CycleCompletionInterval = Settings.G15.TimeFormatsCycleInterval;
            s_lcd.ShowSystemTime = Settings.G15.ShowSystemTime;
            s_lcd.ShowEVETime = Settings.G15.ShowEVETime;

            // Characters names
            List<CCPCharacter> lcdCharacters = new List<CCPCharacter>();

            foreach (Character character in EveClient.MonitoredCharacters.Where(x => x is CCPCharacter))
            {
                lcdCharacters.Add(character as CCPCharacter);
            }

            s_lcd.Characters = lcdCharacters.ToArray();

            // First character to complete a skill
            var nextChar = EveClient.MonitoredCharacters.Where(x => x.IsTraining).ToArray().OrderBy(x => x.CurrentlyTrainingSkill.EndTime).FirstOrDefault();
            if (nextChar != null)
            {
                s_lcd.FirstSkillCompletionRemaingTime = nextChar.CurrentlyTrainingSkill.RemainingTime;
                s_lcd.FirstCharacterToCompleteSkill = nextChar;
            }
        }
        
        #endregion


        #region Event Handlers

        /// <summary>
        /// On every second, we check whether we should start ot stop the LCD display, updated its data, etc.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        static void EveClient_TimerTick(object sender, EventArgs e)
        {
            UpdateOnTimerTick();
        }

        /// <summary>
        /// When skills are completed, we display a special message
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private static void EveClient_QueuedSkillsCompleted(object sender, QueuedSkillsEventArgs e)
        {
            if (s_running)
            {
                if (e.CompletedSkills.Count == 1)
                {
                    s_lcd.SkillCompleted(e.Character);
                }
                else
                {
                    s_lcd.SkillCompleted(e.Character, e.CompletedSkills.Count);
                }
            }
        }

        #endregion


        #region Events triggered by the G15 buttons

        /// <summary>
        /// Occurs whenever the current character changed (because of a button press or cycling).
        /// </summary>
        static void Lcdisplay_CurrentCharacterChanged(Character character)
        {
            Dispatcher.Invoke(() => UpdateOnTimerTick());
        }

        /// <summary>
        /// Occurs whenever a G15 button has been pressed which requires EVEMon to requery the API for the specified character.
        /// </summary>
        static void Lcdisplay_APIUpdateRequested(Character character)
        {
            Dispatcher.Invoke(() =>
                {
                    var ccpCharacter = character as CCPCharacter;
                    if (ccpCharacter != null)
                        ccpCharacter.QueryMonitors.QueryEverything();
                });
        }

        /// <summary>
        /// Occurs whenever the auto cycle should change (because of a button press).
        /// </summary>
        static void Lcdisplay_AutoCycleChanged(bool cycle)
        {
            Settings.G15.UseCharactersCycle = cycle;
        }

        #endregion
    }
}
