using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Threading
{
    /// <summary>
    /// A timer bound to an actor, whose ticking callbacks are invoked on this actor.
    /// </summary>
    public interface IActorTimer : IDisposable
    {
        /// <summary>
        /// Gets or sets the timer's period, in milliseconds.
        /// </summary>
        int Period { get; set; }
        /// <summary>
        /// Starts or resumes the timer.
        /// </summary>
        void Start();
        /// <summary>
        /// Pauses the timer, it will be resumable with <see cref="Start"/>.
        /// </summary>
        void Stop();
    }
}
