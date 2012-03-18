using System;
namespace EVEMon.Common.Threading
{
    /// <summary>
    /// Represents a pseudo-actor pattern, i.e. an object with thread affinity (operations on its members must be done on the underlying thread) and a message pump.
    /// </summary>
    public interface IActor
    {
        /// <summary>
        /// Invoke the provided delegate on the bound thread and immediately returns without waiting for the completion. Note that, when the calling thread and the bound thread are the same, we execute the action immediately without waiting.
        /// </summary>
        /// <param name="action">The action to invoke</param>
        bool BeginInvoke(Action action);

        /// <summary>
        /// Invoke the provided delegate on the bound thread and wait for completion
        /// </summary>
        /// <param name="action">The action to invoke</param>
        bool Invoke(Action action);

        /// <summary>
        /// Asserts the calling thread is this actor's underlying thread or throws an exception
        /// </summary>
        /// <exception cref="AccessViolationException">The calling thread is different from the underlying thread</exception>
        void AssertAccess();

        /// <summary>
        /// Forces the bound thread to shut down in a clean way (no thread abortion)
        /// </summary>
        void Shutdown();

        /// <summary>
        /// Gets a timer.
        /// </summary>
        /// <param name="callback">The callback invoked on every tick.</param>
        /// <param name="period">The ticking period.</param>
        /// <param name="start">When true, the timer starts on creation.</param>
        /// <returns>The created timer.</returns>
        IActorTimer GetTimer(Action callback, int period, bool start);

        /// <summary>
        /// Gets true if the calling thread is the underlying thread; false otherwise.
        /// When false, operations on this actor have to be done through <see cref="Invoke"/> or <see cref="BeginInvoke"/>.
        /// </summary>
        bool HasAccess { get; }

        /// <summary>
        /// Gets true if the actor has been shutdown
        /// </summary>
        bool IsShutdown { get; }
    }
}
