using System;
using System.Collections.Concurrent;
using Pyxis.Publishing;

namespace PyxNet.Pyxis
{
    public static class StackExtensions
    {
        private static ConcurrentDictionary<Stack, Channel> s_associatedChannels = new ConcurrentDictionary<Stack,Channel>();

        public static Channel GetChannel(this Stack stack)
        {
            Channel channel = null;
            if (s_associatedChannels.TryGetValue(stack,out channel)) {
                return channel;
            }

            throw new InvalidOperationException("No channel associated with the stack.");
        }

        public static void AssignUser(this Stack stack,User user)
        {
            AttachChannel(stack, Channel.Authenticate(user));

            var certificateProvider = new LSCertificateProvider(user, stack);
            stack.CertificateProvider = certificateProvider;
        }

        /// <summary>
        /// Attach a channel to the stack to ensure we connect to the proper license server. This
        /// method or AssignUser() must be called before GetChannel()
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="channel">The channel.</param>
        public static void AttachChannel(this Stack stack, Channel channel)
        {
            var lsCertificateValidator = new LSCertificateValidator();
            lsCertificateValidator.SetTrustedAuthorities(channel);

            s_associatedChannels[stack] = channel;
            stack.CertificateValidator = lsCertificateValidator;
        }
    }
}
